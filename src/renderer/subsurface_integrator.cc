#define SPICA_API_EXPORT
#include "subsurface_integrator.h"

#include <iostream>
#include <fstream>
#include <typeinfo>

#include "../core/common.h"

#include "renderer_helper.h"
#include "render_parameters.h"

#include "../core/sampler.h"
#include "../bsdf/bsdf.h"
#include "../bsdf/bssrdf.h"
#include "../random/random_sampler.h"

namespace spica {

    SubsurfaceIntegrator::Octree::Octree()
        : _root(NULL)
        , _numCopies(NULL)
        , _parent(NULL) {
    }

    SubsurfaceIntegrator::Octree::~Octree() {
        release();
    }

    SubsurfaceIntegrator::Octree::Octree(const Octree& octree)
        : _root(NULL)
        , _numCopies(NULL)
        , _parent(NULL) {
        this->operator=(octree);
    }

    SubsurfaceIntegrator::Octree&
    SubsurfaceIntegrator::Octree::operator=(const Octree& octree) {
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

    void SubsurfaceIntegrator::
         Octree::deleteNode(SubsurfaceIntegrator::OctreeNode* node) {
        if (node != NULL) {
            for (int i = 0; i < 8; i++) {
                deleteNode(node->children[i]);
            }
            delete node;
        }
    }

    void SubsurfaceIntegrator::
         Octree::construct(SubsurfaceIntegrator* parent,
                           std::vector<IrradiancePoint>& ipoints) {
        release();

        this->_parent = parent;
        const int numHitpoints = static_cast<int>(ipoints.size());
        
        BBox bbox;
        for (int i = 0; i < numHitpoints; i++) {
            bbox.merge(ipoints[i].pos);
        }

        _root = constructRec(ipoints, bbox);
    }

    SubsurfaceIntegrator::OctreeNode*
    SubsurfaceIntegrator::
    Octree::constructRec(std::vector<IrradiancePoint>& ipoints,
                         const BBox& bbox) {
        if (ipoints.empty()) {
            return NULL;
        } else if (ipoints.size() == 1) {
            OctreeNode* node = new OctreeNode();
            node->pt = ipoints[0];
            node->bbox = bbox;
            node->isLeaf = true;
            return node;
        }

        Vector3D posMid = (bbox.posMin() + bbox.posMax()) * 0.5;

        const int numPoints = static_cast<int>(ipoints.size());
        std::vector<std::vector<IrradiancePoint> > childPoints(8);
        for (int i = 0; i < numPoints; i++) {
            const Vector3D& v = ipoints[i].pos;
            int id = (v.x() < posMid.x() ? 0 : 4) + 
                     (v.y() < posMid.y() ? 0 : 2) + 
                     (v.z() < posMid.z() ? 0 : 1);
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
        node->pt.pos = Vector3D(0.0, 0.0, 0.0);
        node->pt.normal = Vector3D(0.0, 0.0, 0.0);
        node->pt.area = 0.0;

        double weight = 0.0;
        int childCount = 0;
        for (int i = 0; i < 8; i++) {
            if (node->children[i] != nullptr) {
                const double w = node->children[i]->pt.irad.luminance();
                node->pt.pos += w * node->children[i]->pt.pos;
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

    Color SubsurfaceIntegrator::
          Octree::iradSubsurface(const Vector3D& pos,
                                 const BSSRDF& bssrdf) const {
        return iradSubsurfaceRec(_root, pos, bssrdf);
    }

    Color SubsurfaceIntegrator::
          Octree::iradSubsurfaceRec(OctreeNode* node,
                                    const Vector3D& pos,
                                    const BSSRDF& bssrdf) const {
        if (node == NULL) return Color(0.0, 0.0, 0.0);

        const double distSquared = (node->pt.pos - pos).squaredNorm();
        double dw = node->pt.area / distSquared;
        if (node->isLeaf ||
            (dw < _parent->_maxError && !node->bbox.inside(pos))) {
            return bssrdf(pos, node->pt.pos) * (node->pt.irad) * node->pt.area;
        } else {
            Color ret(0.0, 0.0, 0.0);
            for (int i = 0; i < 8; i++) {
                if (node->children[i] != NULL) {
                    ret += iradSubsurfaceRec(node->children[i], pos, bssrdf);
                }
            }
            return ret;
        }
    }

    SubsurfaceIntegrator::SubsurfaceIntegrator()
        : _octree()
        , _photonMap()
        , _dA(0.0)
        , _radius()
        , _triangles() {
    }

    SubsurfaceIntegrator::~SubsurfaceIntegrator() {
    }

    void SubsurfaceIntegrator::initialize(const Scene& scene,
                                          const double maxError) {
        // Extract triangles with BSSRDF
        double avgArea = 0.0;
        _triangles.clear();
        for (int i = 0; i < scene.numTriangles(); i++) {
            if (scene.getBsdf(i).type() & BsdfType::Bssrdf) {
                const Triangle& tri = scene.getTriangle(i);
                _triangles.push_back(tri);
                avgArea += tri.area();
            }
        }

        // Compute dA and copy maxError
        _radius   = sqrt(avgArea / _triangles.size());
        _dA       = (0.5 * _radius) * (0.5 * _radius) * PI;
        _maxError = maxError;
    }

    void SubsurfaceIntegrator::construct(const Scene& scene,
                                         const RenderParameters& params) {
        // If there are no scattering triangle, do nothing
        if (_triangles.empty()) return;

        // Poisson disk sampling
        std::vector<Vector3D> points;
        std::vector<Vector3D> normals;
        sampler::poissonDisk(_triangles, _radius, &points, &normals);

        // Cast photons to compute irradiance at sample points
        _photonMap.construct(scene, params, BsdfType::Bssrdf);

        // Compute irradiance at sample points
        buildOctree(points, normals, params);
    }

    void SubsurfaceIntegrator::buildOctree(const std::vector<Vector3D>& points,
                                           const std::vector<Vector3D>& normals,
                                           const RenderParameters& params) {
        // Compute irradiance on each sampled point
        const int numPoints = static_cast<int>(points.size());
        std::vector<Color> irads(numPoints);

        for(int i = 0; i < numPoints; i++) {
            // Estimate irradiance with photon map
            Color irad = _photonMap.evaluate(points[i], normals[i],
                                             params.gatherPhotons(),
                                             params.gatherRadius());
            irads[i] = irad;
        }

        // Octree construction
        std::vector<IrradiancePoint> iradPoints(numPoints);
        for (int i = 0; i < numPoints; i++) {
            iradPoints[i].pos = points[i];
            iradPoints[i].normal = normals[i];
            iradPoints[i].area = _dA;
            iradPoints[i].irad = irads[i];
        }
        _octree.construct(this, iradPoints);
        std::cout << "Octree constructed !!" << std::endl;
    }

    Color SubsurfaceIntegrator::irradiance(const Vector3D& p,
                                           const BSDF& bsdf) const {
        Assertion(bsdf._bssrdf != NULL,
                  "Specified object does not have BSSRDF !!");
        const Color Mo = _octree.iradSubsurface(p, *bsdf._bssrdf);
        return Color(INV_PI * (1.0 - bsdf._bssrdf->Fdr()) * Mo);
    }

}  // namespace spica

