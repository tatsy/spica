#define SPICA_API_EXPORT
#include "hierarchical.h"

#include <iostream>
#include <fstream>
#include <typeinfo>

#include "../core/common.h"

#include "renderer_helper.h"
#include "render_parameters.h"

#include "../core/sampling.h"
#include "../scenes/scene.h"
#include "../bxdf/bsdf.h"
#include "../bxdf/bssrdf.h"
#include "../random/sampler.h"

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
            _numCopies = NULL;
        }
        else {
            (*_numCopies) -= 1;
        }
    }
}

void HierarchicalIntegrator::
        Octree::deleteNode(HierarchicalIntegrator::OctreeNode* node) {
    if (node != NULL) {
        for (int i = 0; i < 8; i++) {
            deleteNode(node->children[i]);
        }
        delete node;
    }
}

void HierarchicalIntegrator::
        Octree::construct(HierarchicalIntegrator* parent,
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
        return NULL;
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

Spectrum HierarchicalIntegrator::
        Octree::iradSubsurface(const Point3d& pos,
                                const BSSRDF& bssrdf) const {
    return iradSubsurfaceRec(_root, pos, bssrdf);
}

Spectrum HierarchicalIntegrator::
        Octree::iradSubsurfaceRec(OctreeNode* node,
                                  const Point3d& pos,
                                  const BSSRDF& bssrdf) const {
    if (node == NULL) return Spectrum(0.0, 0.0, 0.0);

    const double distSquared = (node->pt.pos - pos).squaredNorm();
    double dw = node->pt.area / distSquared;
    if (node->isLeaf ||
        (dw < _parent->_maxError && !node->bbox.inside(pos))) {
        return bssrdf.S(pos, node->pt.pos) * (node->pt.irad) * node->pt.area;
    } else {
        Spectrum ret(0.0, 0.0, 0.0);
        for (int i = 0; i < 8; i++) {
            if (node->children[i] != NULL) {
                ret += iradSubsurfaceRec(node->children[i], pos, bssrdf);
            }
        }
        return ret;
    }
}

HierarchicalIntegrator::HierarchicalIntegrator()
    : octree_()
    , _photonMap()
    , dA_(0.0)
    , radius_()
    , triangles_() {
}

HierarchicalIntegrator::~HierarchicalIntegrator() {
}

void HierarchicalIntegrator::initialize(const Scene& scene,
                                      const double maxError) {
    // Extract triangles with BSSRDF
    triangles_ = scene.triangulate();
    double avgArea = 0.0;
    for (const auto& t : triangles_) {
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

void HierarchicalIntegrator::construct(const Scene& scene,
                                        const RenderParameters& params) {
    // If there are no scattering triangle, do nothing
    if (_triangles.empty()) return;

    // Poisson disk sampling
    std::vector<Point3d> points;
    std::vector<Normal3d> normals;
    sampler::poissonDisk(_triangles, _radius, &points, &normals);

    // Cast photons to compute irradiance at sample points
    _photonMap.construct(scene, params, BsdfType::Bssrdf);

    // Compute irradiance at sample points
    buildOctree(points, normals, params);
}

void HierarchicalIntegrator::buildOctree(const std::vector<Point>& points,
                                       const std::vector<Normal>& normals,
                                       const RenderParameters& params) {
    // Compute irradiance on each sampled point
    const int numPoints = static_cast<int>(points.size());
    std::vector<Spectrum> irads(numPoints);

    for(int i = 0; i < numPoints; i++) {
        // Estimate irradiance with photon map
        Spectrum irad = _photonMap.evaluate(points[i], normals[i],
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

Spectrum HierarchicalIntegrator::irradiance(const Point3d& p,
                                          const BSDF& bsdf) const {
    Assertion(bsdf._bssrdf != NULL,
                "Specified object does not have BSSRDF !!");
    const Spectrum Mo = _octree.iradSubsurface(p, *bsdf._bssrdf);
    return Spectrum(INV_PI * (1.0 - bsdf._bssrdf->Fdr()) * Mo);
}

}  // namespace spica

