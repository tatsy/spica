#include "subsurface_integrator.h"

#include <iostream>
#include <fstream>

#include "renderer_helper.h"
#include "../utils/sampler.h"

namespace spica {

    SubsurfaceIntegrator::Octree::Octree()
        : _root(NULL)
        , _numCopies(NULL)
        , _parent(NULL)
    {
    }

    SubsurfaceIntegrator::Octree::~Octree()
    {
        release();
    }

    SubsurfaceIntegrator::Octree::Octree(const Octree& octree)
        : _root(NULL)
        , _numCopies(NULL)
        , _parent(NULL)
    {
        this->operator=(octree);
    }

    SubsurfaceIntegrator::Octree& SubsurfaceIntegrator::Octree::operator=(const Octree& octree) {
        release();

        _numCopies = octree._numCopies;
        (*_numCopies) += 1;
        _root = octree._root;
        _parent = octree._parent;

        return *this;
    }

    void SubsurfaceIntegrator::Octree::release() {
        if (_numCopies != NULL) {
            if ((*_numCopies) == 0) {
                deleteNode(_root);
                delete _numCopies;
                _numCopies = NULL;
            }
            else {
                (*_numCopies) -= 1;
            }
        }
    }

    void SubsurfaceIntegrator::Octree::deleteNode(SubsurfaceIntegrator::OctreeNode* node) {
        if (node != NULL) {
            for (int i = 0; i < 8; i++) {
                deleteNode(node->children[i]);
            }
            delete node;
        }
    }

    void SubsurfaceIntegrator::Octree::construct(SubsurfaceIntegrator* parent, std::vector<IrradiancePoint>& ipoints) {
        this->_parent = parent;

        const int numHitpoints = static_cast<int>(ipoints.size());
        
        BBox bbox;
        for (int i = 0; i < numHitpoints; i++) {
            bbox.merge(ipoints[i].pos);
        }

        _root = constructRec(ipoints, bbox);
    }

    SubsurfaceIntegrator::OctreeNode* SubsurfaceIntegrator::Octree::constructRec(std::vector<IrradiancePoint>& ipoints, const BBox& bbox) {
        if (ipoints.empty()) {
            return NULL;
        } else if (ipoints.size() == 1) {
            OctreeNode* node = new OctreeNode();
            node->pt = ipoints[0];
            node->bbox = bbox;
            node->isLeaf = true;
            return node;
        }

        Vector3 posMid = (bbox.posMin() + bbox.posMax()) * 0.5;

        const int numPoints = static_cast<int>(ipoints.size());
        std::vector<std::vector<IrradiancePoint> > childPoints(8);
        for (int i = 0; i < numPoints; i++) {
            const Vector3& v = ipoints[i].pos;
            int id = (v.x() < posMid.x() ? 0 : 4) + (v.y() < posMid.y() ? 0 : 2) + (v.z() < posMid.z() ? 0 : 1);
            childPoints[id].push_back(ipoints[i]);
        }

        // Compute child nodes
        OctreeNode* node = new OctreeNode();
        for (int i = 0; i < 8; i++) {
            BBox childBox;
            for (int j = 0; j < childPoints[i].size(); j++) {
                childBox.merge(childPoints[i][j].pos);
            }
            node->children[i] = constructRec(childPoints[i], childBox);
        }
        node->bbox = bbox;
        node->isLeaf = false;

        // Accumulate child nodes
        node->pt.pos = Vector3(0.0, 0.0, 0.0);
        node->pt.normal = Vector3(0.0, 0.0, 0.0);
        node->pt.area = 0.0;

        double weight = 0.0;
        int childCount = 0;
        for (int i = 0; i < 8; i++) {
            if (node->children[i] != NULL) {
                double w = node->children[i]->pt.irad.luminance();
                node->pt.pos = w * node->children[i]->pt.pos;
                node->pt.normal += w * node->children[i]->pt.normal;
                node->pt.area += node->children[i]->pt.area;
                node->pt.irad += node->children[i]->pt.irad;
                weight += w;
                childCount += 1;
            }
        }

        if (weight > 0.0) {
            node->pt.pos    /= weight;
            node->pt.normal /= weight;
        }

        if (childCount != 0) {
            node->pt.irad /= childCount;
        }

        return node;
    }

    Color SubsurfaceIntegrator::Octree::iradSubsurface(const Vector3& pos, const DiffusionReflectance& Rd) const {
        return iradSubsurfaceRec(_root, pos, Rd);
    }

    Color SubsurfaceIntegrator::Octree::iradSubsurfaceRec(OctreeNode* node, const Vector3& pos, const DiffusionReflectance& Rd) const {
        if (node == NULL) return Color(0.0, 0.0, 0.0);

        const double distSquared = (node->pt.pos - pos).squaredNorm();
        double dw = node->pt.area / distSquared;
        if (node->isLeaf || (dw < _parent->bssrdf.maxError() && !node->bbox.inside(pos))) {
            return Rd(distSquared) * node->pt.irad * node->pt.area;
        } else {
            Color ret(0.0, 0.0, 0.0);
            for (int i = 0; i < 8; i++) {
                if (node->children[i] != NULL) {
                    ret += iradSubsurfaceRec(node->children[i], pos, Rd);
                }
            }
            return ret;
        }
    }



    SubsurfaceIntegrator::SubsurfaceIntegrator()
        : mtrl()
        , bssrdf()
    {
    }

    SubsurfaceIntegrator::~SubsurfaceIntegrator()
    {
    }

    void SubsurfaceIntegrator::init(const Scene& scene, const Camera& camera, const BSSRDF& bssrdf_, const double areaRadius, std::vector<HitpointInfo>* hpoints) {
        // Poisson disk sampling on SSS objects
        int objectID = -1;
        std::vector<Vector3> points;
        std::vector<Vector3> normals;
        for (int i = 0; i < scene.numObjects(); i++) {
            if (scene.getMaterial(i).reftype == REFLECTION_SUBSURFACE) {
                msg_assert(points.empty(), "# of objects with subsurface scattering property must be only one !!");

                const Primitive* obj = scene.get(i);
                std::string typname = typeid(*obj).name();
                msg_assert(typname == "class spica::Trimesh", "Object with subsurface scattering property must be Trimesh !!");

                const Trimesh* trimesh = reinterpret_cast<const Trimesh*>(obj);
                sampler::poissonDisk(*trimesh, areaRadius, &points, &normals);
                objectID = i;
            }
        }
        msg_assert(objectID >= 0, "The scene does not have subsurface scattering object!!");

        // Copy material data
        this->mtrl = scene.getMaterial(objectID);
        this->bssrdf = bssrdf_;

        // Store points on SSS object
        const int numPoints = static_cast<int>(points.size());
        for (int i = 0; i < numPoints; i++) {
            HitpointInfo hp(points[i], normals[i]);
            hp.weight = Color(1.0, 1.0, 1.0);
            hp.coeff  = 1.0;
            hpoints->push_back(hp);
        }
    }


    void SubsurfaceIntegrator::buildOctree(const std::vector<HitpointInfo>& hpoints, const int numShotPhotons, const double areaRadius) {
        // Compute irradiance on each sampled point
        const int numPoints = static_cast<int>(hpoints.size());
        std::vector<Color> irads(numPoints);

        for(int i = 0; i < numPoints; i++) {
            // Estimate irradiance with photon map
            const HitpointInfo& hp = hpoints[i];
            Color irad = (hp.emission + hp.flux / (PI * hp.r2)) * (hp.coeff / numShotPhotons);
            irads[i] = irad.cwiseMultiply(mtrl.color);
        }

        // Save radiance data for visual checking
        std::ofstream ofs("sss_sppm_irads.obj", std::ios::out);
        for (int i = 0; i < numPoints; i++) {
            Vector3 p = hpoints[i];
            Vector3 clr = Vector3::minimum(irads[i], Vector3(1.0, 1.0, 1.0));
            ofs << "v " <<  p.x() << " " << p.y() << " " << p.z();
            ofs << " " << clr.x() << " " << clr.y() << " " << clr.z() << std::endl;
        }
        ofs.close();

        // Octree construction
        std::vector<IrradiancePoint> iradPoints(numPoints);
        for (int i = 0; i < numPoints; i++) {
            iradPoints[i].pos = static_cast<Vector3>(hpoints[i]);
            iradPoints[i].normal = hpoints[i].normal;
            iradPoints[i].area = PI * (0.5 * areaRadius) * (0.5 * areaRadius);
            iradPoints[i].irad = irads[i];
        }
        octree.construct(this, iradPoints);
        std::cout << "Octree constructed !!" << std::endl;
    }

    Color SubsurfaceIntegrator::irradiance(const Vector3& p) const {
        DiffusionReflectance Rd(bssrdf.sigma_a(), bssrdf.sigmap_s(), bssrdf.eta());
        Color Mo = octree.iradSubsurface(p, Rd);
        return (1.0 / PI) * (1.0 - Rd.Fdr(bssrdf.eta())) * Mo;
    }

}  // namespace spica

