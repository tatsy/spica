#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SPPM_H_
#define _SPICA_SPPM_H_

#include "core/common.h"
#include "core/cobject.h"
#include "core/hash_grid.h"
#include "core/render.hpp"
#include "core/integrator.h"

namespace spica {

/** Stochastic progressive photon mapping
 *  @ingroup renderer_module
 */
class SPICA_EXPORTS SPPMIntegrator : public Integrator {
public:
    // Public methods
    SPPMIntegrator(const std::shared_ptr<Sampler>& sampler);

    SPPMIntegrator(RenderParams &params);

    ~SPPMIntegrator();

    void render(const std::shared_ptr<const Camera>& camera,
                const Scene& scene,
                RenderParams& params) override;
        
private:
    // Private internal class
    struct SPPMPixel;

    // 1st pass: Trace rays from camera
    void traceRays(const std::shared_ptr<const Camera> &camera,
                   const Scene& scene,
                   RenderParams& params,
                   const std::vector<std::unique_ptr<Sampler>>& samplers,
                   std::vector<MemoryArena>& arenas,
                   std::vector<SPPMPixel>& hpoints) const;

    // 2nd pass: Trace photons from lights
    void tracePhotons(const Scene& scene,
                      RenderParams& params,
                      const std::vector<std::unique_ptr<Sampler>>& samplers,
                      std::vector<MemoryArena>& arenas,
                      const Distribution1D& lightDistrib,
                      const int numPhotons) const;

    void tracePhotonsSub(const Scene& scene,
                         RenderParams& params,
                         const Ray& r,
                         const Spectrum& b,
                         Sampler& sampler,
                         MemoryArena& arena) const;

    void pathTrace(const Scene& scene, 
                   RenderParams& params,
                   const Ray& ray,
                   Sampler& sampler,
                   MemoryArena& arena,
                   SPPMPixel* hp) const;

    void constructHashGrid(std::vector<SPPMPixel>& hpoints,
                           int imageW, int imageH) const;

    // Private fields
    std::shared_ptr<Sampler> sampler_;
    mutable HashGrid<SPPMPixel*> hashgrid_;
    static const double kAlpha_;

};  // class SPPMIntegrator

SPICA_EXPORT_PLUGIN(SPPMIntegrator, "Stochastic progressive photon mapping");

}  // namespace spica

#endif  // _SPICA_SPPM_H_
