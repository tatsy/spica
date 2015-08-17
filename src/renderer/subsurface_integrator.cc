#define SPICA_SUBSURFACE_INTEGRATOR_EXPORT
#include "subsurface_integrator.h"

#include <iostream>
#include <fstream>
#include <typeinfo>

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

        Vector3D posMid = (bbox.posMin() + bbox.posMax()) * 0.5;

        const int numPoints = static_cast<int>(ipoints.size());
        std::vector<std::vector<IrradiancePoint> > childPoints(8);
        for (int i = 0; i < numPoints; i++) {
            const Vector3D& v = ipoints[i].pos;
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
        node->pt.pos = Vector3D(0.0, 0.0, 0.0);
        node->pt.normal = Vector3D(0.0, 0.0, 0.0);
        node->pt.area = 0.0;

        double weight = 0.0;
        int childCount = 0;
        for (int i = 0; i < 8; i++) {
            if (node->children[i] != NULL) {
                double w = node->children[i]->pt.irad.luminance();
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

    Color SubsurfaceIntegrator::Octree::iradSubsurface(const Vector3D& pos, const BSSRDF& bssrdf) const {
        return iradSubsurfaceRec(_root, pos, bssrdf);
    }

    Color SubsurfaceIntegrator::Octree::iradSubsurfaceRec(OctreeNode* node, const Vector3D& pos, const BSSRDF& bssrdf) const {
        if (node == NULL) return Color(0.0, 0.0, 0.0);

        const double distSquared = (node->pt.pos - pos).squaredNorm();
        double dw = node->pt.area / distSquared;
        if (node->isLeaf || (dw < _parent->_maxError && !node->bbox.inside(pos))) {
            return Color(bssrdf(distSquared).multiply(node->pt.irad) * node->pt.area);
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
        : mtrl()
        , bssrdf()
        , octree()
        , photonMap()
        , dA(0.0)
    {
    }

    SubsurfaceIntegrator::~SubsurfaceIntegrator()
    {
    }

    void SubsurfaceIntegrator::initialize(const Scene& scene, const BSSRDF& bssrdf_, const PMParams& params, const double areaRadius, const RandomType randType, const double maxError) {
        // Poisson disk sampling on SSS objects
        int objectID = -1;
        std::vector<Vector3D> points;
        std::vector<Vector3D> normals;
        for (int i = 0; i < scene.numObjects(); i++) {
            if (scene.getMaterial(i).reftype == REFLECTION_SUBSURFACE) {
                Assertion(points.empty(), "# of objects with subsurface scattering property must be only one !!");

                const IGeometry* obj = scene.get(i);
                Assertion(typeid(*obj) == typeid(Trimesh), "Object with subsurface scattering property must be Trimesh !!");

                const Trimesh* trimesh = reinterpret_cast<const Trimesh*>(obj);
                sampler::poissonDisk(*trimesh, areaRadius, &points, &normals);
                objectID = i;
            }
        }
        Assertion(objectID >= 0, "The scene does not have subsurface scattering object!!");

        // Copy material data
        this->mtrl = scene.getMaterial(objectID);
        this->bssrdf = bssrdf_;
        this->dA = (0.5 * areaRadius) * (0.5 * areaRadius) * PI;
        this->_maxError = maxError;

        // Cast photons to compute irradiance at sample points
        buildPhotonMap(scene, params.numPhotons, 64, randType);

        // Compute irradiance at sample points
        buildOctree(points, normals, params);
    }


    void SubsurfaceIntegrator::buildOctree(const std::vector<Vector3D>& points, const std::vector<Vector3D>& normals, const PMParams& params) {
        // Compute irradiance on each sampled point
        const int numPoints = static_cast<int>(points.size());
        std::vector<Color> irads(numPoints);

        for(int i = 0; i < numPoints; i++) {
            // Estimate irradiance with photon map
            Color irad = irradianceWithPM(points[i], normals[i], params);
            irads[i] = irad.multiply(mtrl.color);
        }

        // Save radiance data for visual checking
        std::ofstream ofs("sss_sppm_irads.obj", std::ios::out);
        for (int i = 0; i < numPoints; i++) {
            Vector3D p = points[i];
            Vector3D clr = Vector3D::minimum(irads[i], Vector3D(1.0, 1.0, 1.0));
            ofs << "v " <<  p.x() << " " << p.y() << " " << p.z();
            ofs << " " << clr.x() << " " << clr.y() << " " << clr.z() << std::endl;
        }
        ofs.close();

        // Octree construction
        std::vector<IrradiancePoint> iradPoints(numPoints);
        for (int i = 0; i < numPoints; i++) {
            iradPoints[i].pos = points[i];
            iradPoints[i].normal = normals[i];
            iradPoints[i].area = dA;
            iradPoints[i].irad = irads[i];
        }
        octree.construct(this, iradPoints);
        std::cout << "Octree constructed !!" << std::endl;
    }

    Color SubsurfaceIntegrator::irradiance(const Vector3D& p) const {
        Color Mo = octree.iradSubsurface(p, bssrdf);
        return Color((1.0 / PI) * (1.0 - bssrdf.Fdr()) * Mo);
    }

    void SubsurfaceIntegrator::buildPhotonMap(const Scene& scene, const int numPhotons, const int bounceLimit, const RandomType randType) {
        std::cout << "Shooting photons ..." << std::endl;

        RandomBase* rand = NULL;
        switch (randType) {
        case PSEUDO_RANDOM_TWISTER:
            rand = new Random();
            break;
        case QUASI_MONTE_CARLO:
            rand = new Halton();
            break;
        default:
            Assertion(false, "Unknown random number generator type!!");
            break;
        }

        // Shooting photons
        std::vector<Photon> photons;
        int proc = 0;
        ompfor (int pid = 0; pid < numPhotons; pid++) {
            RandomSeq rseq;
            omplock {
                rand->requestSamples(rseq, 200);
            }

            Photon photon = Photon::sample(scene, rseq, numPhotons);

            const Vector3D& lightNormal = photon.normal();
            const Vector3D& lightPos    = static_cast<Vector3D>(photon);
            Color currentFlux = photon.flux();

            const double r1 = rseq.next();
            const double r2 = rseq.next();
            Vector3D nextDir;
            sampler::onHemisphere(lightNormal, &nextDir, r1, r2);
            Ray currentRay(lightPos, nextDir);

            for (int bounce = 0; ; bounce++) {
                std::vector<double> randnums;
                rseq.next(2, &randnums);

                // Remove photon with zero flux
                if (std::max(currentFlux.red(), std::max(currentFlux.green(), currentFlux.blue())) <= 0.0 || bounce > bounceLimit) {
                    break;
                }

                Intersection isect;
                bool isHit = scene.intersect(currentRay, isect);
                if (!isHit) {
                    break;
                }

                const int objectID = isect.objectId();
                const Material& mtrl = scene.getMaterial(objectID);
                const Hitpoint& hitpoint = isect.hitpoint();

                const Vector3D orientNormal = Vector3D::dot(currentRay.direction(), hitpoint.normal()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

                if (mtrl.reftype == REFLECTION_DIFFUSE) {
                    sampler::onHemisphere(orientNormal, &nextDir, randnums[0], randnums[1]);
                    currentRay = Ray(hitpoint.position(), nextDir);
                    currentFlux = currentFlux.multiply(mtrl.color);
                } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                    nextDir = Vector3D::reflect(currentRay.direction(), orientNormal);
                    currentRay = Ray(hitpoint.position(), nextDir);
                    currentFlux = currentFlux.multiply(mtrl.color);
                } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                    bool isIncoming = Vector3D::dot(hitpoint.normal(), orientNormal) > 0.0;

                    Vector3D reflectDir, transmitDir;
                    double fresnelRe, fresnelTr;
                    bool isTotRef = helper::isTotalRef(isIncoming,
                                                        hitpoint.position(),
                                                        currentRay.direction(),
                                                        hitpoint.normal(),
                                                        orientNormal,
                                                        &reflectDir,
                                                        &transmitDir,
                                                        &fresnelRe,
                                                        &fresnelTr);


                    Ray reflectRay = Ray(hitpoint.position(), reflectDir);

                    if (isTotRef) {
                        // Total reflection
                        currentRay = reflectRay;
                        currentFlux = currentFlux.multiply(mtrl.color);
                    } else {
                        const double probability = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                        if (randnums[0] < probability) {
                            // Reflection
                            currentRay = reflectRay;
                            currentFlux = currentFlux.multiply(mtrl.color) * (fresnelRe / probability);
                        } else {
                            // Reflaction
                            currentRay = Ray(hitpoint.position(), transmitDir);
                            currentFlux = currentFlux.multiply(mtrl.color) * (fresnelTr / (1.0 - probability));
                        }
                    }
                } else if (mtrl.reftype == REFLECTION_SUBSURFACE) {
                    // Store photon
                    omplock {
                        photons.push_back(Photon(hitpoint.position(), currentFlux, currentRay.direction(), hitpoint.normal()));
                    }
                    break;
                }
            }

            omplock {
                proc++;
                if (proc % 1000 == 0) {
                    printf("%6.2f %% processed ...\r", 100.0 * proc / numPhotons);
                }
            }
        }
        printf("\n\n");

        // Construct photon map
        photonMap.clear();
        photonMap.construct(photons);

        // Release memory
        delete rand;
    }

    Color SubsurfaceIntegrator::irradianceWithPM(const Vector3D& p, const Vector3D& n, const PMParams& params) const {
        // Estimate irradiance with photon map
        Photon query = Photon(p, Color(), Vector3D(), n);
        std::vector<Photon> photons;
        photonMap.findKNN(query, &photons, params.gatherPhotons, params.gatherRadius);

        const int numPhotons = static_cast<int>(photons.size());

        std::vector<Photon> validPhotons;
        std::vector<double> distances;
        double maxdist = 0.0;
        for (int i = 0; i < numPhotons; i++) {
            Vector3D diff = query - photons[i];
            double dist = diff.norm();
            if (std::abs(Vector3D::dot(n, diff)) < diff.norm() * 0.1) {
                validPhotons.push_back(photons[i]);
                distances.push_back(dist);
                maxdist = std::max(maxdist, dist);
            }
        }

        // Cone filter
        const int numValidPhotons = static_cast<int>(validPhotons.size());
        const double k = 1.1;
        Color totalFlux = Color(0.0, 0.0, 0.0);
        for (int i = 0; i < numValidPhotons; i++) {
            const double w = 1.0 - (distances[i] / (k * maxdist));
            const Color v = Color(photons[i].flux() / PI);
            totalFlux += w * v;
        }
        totalFlux /= (1.0 - 2.0 / (3.0 * k));

        if (maxdist > EPS) {
            return Color(totalFlux / (PI * maxdist * maxdist));
        }
        return Color(0.0, 0.0, 0.0);
    }

}  // namespace spica

