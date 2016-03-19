#define SPICA_API_EXPORT
#include "hierarchical.h"

#include <iostream>
#include <fstream>
#include <typeinfo>

#include "../core/common.h"
#include "../core/sampling.h"
#include "../core/timer.h"
#include "../math/vect_math.h"
#include "../scenes/scene.h"
#include "../shape/visibility_tester.h"
#include "../bxdf/bsdf.h"
#include "../bxdf/bssrdf.h"
#include "../material/material.h"
#include "../random/sampler.h"

#include "mis.h"
#include "render_parameters.h"

namespace spica {

HierarchicalIntegrator::Octree::Octree()
    : _root(NULL)
    , _numCopies(NULL)
    , _parent(NULL) {
}

HierarchicalIntegrator::Octree::~Octree() {
    release();
}

HierarchicalIntegrator::Octree::Octree(const Octree& octree)
    : _root(NULL)
    , _numCopies(NULL)
    , _parent(NULL) {
    this->operator=(octree);
}

HierarchicalIntegrator::Octree&
HierarchicalIntegrator::Octree::operator=(const Octree& octree) {
    release();
    _numCopies = octree._numCopies;
    (*_numCopies) += 1;
    _root = octree._root;
    _parent = octree._parent;
    return *this;
}

void HierarchicalIntegrator::Octree::release() {
    if (_numCopies != NULL) {
        if ((*_numCopies) == 0) {
            deleteNode(_root);
            delete _numCopies;
            _numCopies = nullptr;
        }
        else {
            (*_numCopies) -= 1;
        }
    }
}

void HierarchicalIntegrator::Octree::deleteNode(
    HierarchicalIntegrator::OctreeNode* node) {
    if (node != nullptr) {
        for (int i = 0; i < 8; i++) {
            deleteNode(node->children[i]);
        }
        delete node;
        node = nullptr;
    }
}

void HierarchicalIntegrator::Octree::construct(
    HierarchicalIntegrator* parent,
    std::vector<IrradiancePoint>& ipoints) {
    release();

    this->_parent = parent;
    const int numHitpoints = static_cast<int>(ipoints.size());
        
    Bounds3d bbox;
    for (int i = 0; i < numHitpoints; i++) {
        bbox.merge(ipoints[i].pos);
    }

    _root = constructRec(ipoints, bbox);
}

HierarchicalIntegrator::OctreeNode*
HierarchicalIntegrator::Octree::constructRec(
    std::vector<IrradiancePoint>& ipoints,
    const Bounds3d& bbox) {
    if (ipoints.empty()) {
        return nullptr;
    } else if (ipoints.size() == 1) {
        OctreeNode* node = new OctreeNode();
        node->pt = ipoints[0];
        node->bbox = bbox;
        node->isLeaf = true;
        return node;
    }

    Point3d posMid = (bbox.posMin() + bbox.posMax()) * 0.5;

    const int numPoints = static_cast<int>(ipoints.size());
    std::vector<std::vector<IrradiancePoint> > childPoints(8);
    for (int i = 0; i < numPoints; i++) {
        const Point3d& v = ipoints[i].pos;
        int id = (v.x() < posMid.x() ? 0 : 4) + 
                 (v.y() < posMid.y() ? 0 : 2) + 
                 (v.z() < posMid.z() ? 0 : 1);
        childPoints[id].push_back(ipoints[i]);
    }

    // Compute child nodes
    OctreeNode* node = new OctreeNode();
    for (int i = 0; i < 8; i++) {
        Bounds3d childBox;
        for (int j = 0; j < childPoints[i].size(); j++) {
            childBox.merge(childPoints[i][j].pos);
        }
        node->children[i] = constructRec(childPoints[i], childBox);
    }
    node->bbox = bbox;
    node->isLeaf = false;

    // Accumulate child nodes
    node->pt.pos    = Point3d(0.0, 0.0, 0.0);
    node->pt.normal = Normal3d(0.0, 0.0, 0.0);
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

Spectrum HierarchicalIntegrator::Octree::iradSubsurface(
    const SurfaceInteraction& po,
    const std::unique_ptr<DiffusionReflectance>& Rd) const {
    return iradSubsurfaceRec(_root, po, Rd);
}

Spectrum HierarchicalIntegrator::Octree::iradSubsurfaceRec(
    OctreeNode* node,
    const SurfaceInteraction& po,
    const std::unique_ptr<DiffusionReflectance>& Rd) const {
    if (node == nullptr) {
        return Spectrum(0.0);
    }

    double distSquared = (node->pt.pos - po.pos()).squaredNorm();
    double dw = node->pt.area / distSquared;
    if (node->isLeaf || (dw < _parent->maxError_ &&
                         !node->bbox.inside(po.pos()))) {
        const double r = std::sqrt(distSquared);
        return (*Rd)(node->pt.pos, po.pos()) * node->pt.irad * node->pt.area;
    } else {
        Spectrum ret(0.0);
        for (int i = 0; i < 8; i++) {
            if (node->children[i] != nullptr) {
                ret += iradSubsurfaceRec(node->children[i], po, Rd);
            }
        }
        return ret;
    }
}

HierarchicalIntegrator::HierarchicalIntegrator(
    const std::shared_ptr<const Camera>& camera,
    const std::shared_ptr<Sampler>& sampler,
    double maxError)
    : SamplerIntegrator{ camera, sampler }
    , photonmap_{}
    , octree_{}
    , dA_{ 0.0 }
    , radius_{}
    , maxError_{ maxError }
    , triangles_{} {
}

HierarchicalIntegrator::~HierarchicalIntegrator() {
}

Spectrum HierarchicalIntegrator::Li(const Scene& scene,
                                    const RenderParameters& params,
                                    const Ray& r,
                                    Sampler& sampler,
                                    MemoryArena& arena,
                                    int depth) const {
    Ray ray(r);
    Spectrum L(0.0);
    Spectrum beta(1.0);
    bool specularBounce = false;
    int bounces;
    for (bounces = 0; ; bounces++) {
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        // Sample Le which contributes without any loss
        if (bounces == 0 || specularBounce) {
            if (isIntersect) {
                L += beta * isect.Le(-ray.dir());
            } else {
                for (const auto& light : scene.lights()) {
                    L += beta * light->Le(ray);
                }
            }
        }

        if (!isIntersect || bounces >= params.bounceLimit()) break;

        isect.setScatterFuncs(ray, arena);
        if (!isect.bsdf()) {
            ray = isect.spawnRay(ray.dir());
            bounces--;
            continue;
        }

        if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
            Spectrum Ld = beta * mis::uniformSampleOneLight(isect, scene, arena, sampler);
            L += Ld;
        }

        // Process BxDF
        Vector3d wo = -ray.dir();
        Vector3d wi;
        double pdf;
        BxDFType sampledType;
        Spectrum ref = isect.bsdf()->sample(wo, &wi, sampler.get2D(), &pdf,
                                            BxDFType::All, &sampledType);

        if (ref.isBlack() || pdf == 0.0) break;

        beta *= ref * vect::absDot(wi, isect.normal()) / pdf;
        specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
        ray = isect.spawnRay(wi);

        // Account for BSSRDF
        if (isect.bssrdf() && (sampledType & BxDFType::Transmission) != BxDFType::None) {
            L += beta * this->irradiance(isect);
            break;
        }

        // Russian roulette
        if (bounces > 3) {
            double continueProbability = std::min(0.95, beta.luminance());
            if (sampler.get1D() > continueProbability) break;
            beta /= continueProbability;
        }
    }
    return L;
}

void HierarchicalIntegrator::initialize(const Scene& scene,
                                        const RenderParameters& params,
                                        Sampler& sampler) {
    // Extract triangles with BSSRDF
    triangles_.clear();
    const auto& prims = scene.primitives();
    for (const auto& p : prims) {
        if (p->material()->isSubsurface()) {
            const auto& tris = p->triangulate();
            triangles_.insert(triangles_.end(), tris.begin(), tris.end());
        }
    }

    double avgArea = 0.0;
    for (const auto& t : triangles_) {
        avgArea += t.area();
    }
    avgArea /= triangles_.size();

    // Compute dA and copy maxError
    radius_   = std::sqrt(avgArea);
    dA_       = (0.5 * radius_) * (0.5 * radius_) * PI;

    // Construct octree
    construct(scene, sampler, params);
}

void HierarchicalIntegrator::startNextLoop(const Scene& scene,
                                           const RenderParameters& params,
                                           Sampler& sampler) {
    // Construct octree
    construct(scene, sampler, params);
}

void HierarchicalIntegrator::construct(const Scene& scene,
                                       Sampler& sampler,
                                       const RenderParameters& params) {
    // If there are no scattering triangle, do nothing
    if (triangles_.empty()) return;

    // Poisson disk sampling
    std::vector<Interaction> points;
    samplePoissonDisk(triangles_, radius_, &points);
    MsgInfo("%zu points sampled with PDS.", points.size());

    // Construct photon map
    photonmap_.construct(scene, params);

    // Compute irradiance at sample points
    buildOctree(scene, points, params);
}

void HierarchicalIntegrator::buildOctree(const Scene& scene,
                                         const std::vector<Interaction>& points,
                                         const RenderParameters& params) {
    // Compute irradiance on each sampled point
    const int numPoints = static_cast<int>(points.size());
    std::vector<Spectrum> irads(numPoints);

    for(int i = 0; i < numPoints; i++) {
        // Estimate irradiance with photon map
        irads[i] = photonmap_.evaluateE(points[i], params.gatherPhotons(),
                                        params.gatherRadius());
    }

    // Octree construction
    std::vector<IrradiancePoint> iradPoints(numPoints);
    for (int i = 0; i < numPoints; i++) {
        iradPoints[i].pos    = points[i].pos();
        iradPoints[i].normal = points[i].normal();
        iradPoints[i].area = dA_;
        iradPoints[i].irad = irads[i];
    }
    octree_.construct(this, iradPoints);
    std::cout << "Octree constructed !!" << std::endl;
}

Spectrum HierarchicalIntegrator::irradiance(const SurfaceInteraction& po) const {
    Assertion(po.bssrdf(), "BSSRDF not found!!");
    auto Rd = static_cast<SeparableBSSRDF*>(po.bssrdf())->Rd();
    const Spectrum Mo = octree_.iradSubsurface(po, Rd);
    return (INV_PI * (1.0 - Rd->Ft(po.wo())) * (1.0 - Rd->Fdr())) * Mo;
}

}  // namespace spica

