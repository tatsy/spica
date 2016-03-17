#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SUBSURFACE_INTEGRATOR_H_
#define _SPICA_SUBSURFACE_INTEGRATOR_H_

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/bounds3d.h"

#include "integrator.h"
#include "photon_map.h"

namespace spica {

struct IrradiancePoint {
    Point3d  pos;
    Normal3d normal;
    double area;
    Spectrum irad;

    IrradiancePoint()
        : pos()
        , normal()
        , area(0.0)
        , irad()
    {
    }

    IrradiancePoint(const Point3d& pos_, const Normal3d& normal_,
                    const double area_, const Spectrum& irad_)
        : pos(pos_)
        , normal(normal_)
        , area(area_)
        , irad(irad_)
    {
    }

    IrradiancePoint(const IrradiancePoint& p)
        : pos()
        , normal()
        , area(0.0)
        , irad()
    {
        this->operator=(p);
    }

    IrradiancePoint& operator=(const IrradiancePoint& p) {
        this->pos = p.pos;
        this->normal = p.normal;
        this->area = p.area;
        this->irad = p.irad;
        return *this;
    }
};

/** Irradiance integrator for subsurface scattering objects
 *  @ingroup renderer_module
 */
class SPICA_EXPORTS HierarchicalIntegrator : public SamplerIntegrator {
public:
    // Public methods
    HierarchicalIntegrator(const std::shared_ptr<const Camera>& camera,
                           const std::shared_ptr<Sampler>& smapler,
                           double maxError = 0.05);
    ~HierarchicalIntegrator();

    void initialize(const Scene& scene,
                    const RenderParameters& params,
                    Sampler& sampler) override;

    Spectrum Li(const Scene& scene,
                const RenderParameters& params,
                const Ray& ray,
                Sampler& sampler,
                MemoryArena& arena,
                int depth = 0) const override;

private:
    // Private internal classes
    struct OctreeNode {
        IrradiancePoint pt;
        Bounds3d bbox;
        OctreeNode* children[8];
        bool isLeaf;

        OctreeNode()
            : pt{}
            , bbox{}
            , isLeaf{ false } {
            for (int i = 0; i < 8; i++) {
                children[i] = nullptr;
            }
        }                
    };

    class Octree {
    private:
        OctreeNode* _root;
        int* _numCopies;
        HierarchicalIntegrator* _parent;

    public:
        Octree();
        ~Octree();

        Octree(const Octree& octree);
        Octree& operator=(const Octree& octree);

        void construct(HierarchicalIntegrator* parent,
                        std::vector<IrradiancePoint>& ipoints);

        Spectrum iradSubsurface(const SurfaceInteraction& intr) const;

    private:
        void release();
        void deleteNode(OctreeNode* node);
        OctreeNode* constructRec(std::vector<IrradiancePoint>& pointers,
                                    const Bounds3d& bbox);

        Spectrum iradSubsurfaceRec(OctreeNode* node,
                                   const SurfaceInteraction& po) const;
    };

    // Private methods
    void construct(const Scene& scene,
                   Sampler& sampler,
                   const RenderParameters& params);

    Spectrum irradiance(const SurfaceInteraction& po, double eta) const;

    void buildOctree(const Scene& scene,
                     Sampler& sampler,
                     const std::vector<Interaction>& points,
                     const RenderParameters& params);
    
    // Private fields
    PhotonMap photonmap_;
    Octree    octree_;
    double    dA_;
    double    radius_;
    double    maxError_;
    std::vector<Triangle> triangles_;

};  // class HierarchicalIntegrator

}  // namespace spica

#endif  // _SPICA_SUBSURFACE_INTEGRATOR_H_
