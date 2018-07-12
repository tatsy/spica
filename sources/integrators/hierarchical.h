#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SUBSURFACE_INTEGRATOR_H_
#define _SPICA_SUBSURFACE_INTEGRATOR_H_

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/bounds3d.h"
#include "../core/interaction.h"

#include "integrator.h"
#include "photon_map.h"

namespace spica {

/**
 *
 */
class Hierarchy {
public:
    // Public methods
    Hierarchy(double radius, double maxError);
    ~Hierarchy();

    // Private methods
    Spectrum irradiance(const SurfaceInteraction& po) const;

    void samplePoints(const Scene& scene, const Point3d& pCamera);

    void buildOctree(const Scene& scene,
                     const RenderParams& params,
                     Sampler& sampler);

private:
    // Private methods
    Spectrum Li(const Scene& scene,
                const RenderParams& params,
                const Ray& ray,
                Sampler& sampler,
                MemoryArena& arena,
                int depth) const;

    // Private fields
    std::vector<Interaction> points_;
   
    class Octree;
    std::unique_ptr<Octree> octree_;
    double radius_;
    std::unique_ptr<PhotonMap> photonmap_;
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
                    const RenderParams& params,
                    Sampler& sampler) override;

    void loopStarted(const Scene& scene,
                     const RenderParams& params,
                     Sampler& sampler) override;

    Spectrum Li(const Scene& scene,
                const RenderParams& params,
                const Ray& ray,
                Sampler& sampler,
                MemoryArena& arena,
                int depth = 0) const override;

private:
    std::unique_ptr<Hierarchy> hi_;

};  // class HierarchicalIntegrator

}  // namespace spica

#endif  // _SPICA_SUBSURFACE_INTEGRATOR_H_
