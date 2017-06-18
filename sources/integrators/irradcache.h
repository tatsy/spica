#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_IRRADCACHE_H_
#define _SPICA_IRRADCACHE_H_

#include "../core/common.h"
#include "../core/interaction.h"
#include "../core/spectrum.h"

#include "../core/kdtree.h"
#include "integrator.h"

namespace spica {

/**
 * Irradiance caching.
 * @ingroup integrator_module
 */
class SPICA_EXPORTS IrradCacheIntegrator : public SamplerIntegrator {
public:
    // Public methods
    IrradCacheIntegrator(const std::shared_ptr<const Camera>& camera,
                         const std::shared_ptr<Sampler>& sampler);
    ~IrradCacheIntegrator();

    void initialize(const Scene& scene,
                    const RenderParams& params,
                    Sampler& sampler) override;

    void loopStarted(const Scene& scene,
                     const RenderParams& params,
                     Sampler& sampler) override;

    void loopFinished(const Scene& scene,
                      const RenderParams& params,
                      Sampler& sampler) override;

    Spectrum Li(const Scene& scene,
                const RenderParams& params,
                const Ray& ray,
                Sampler& sampler,
                MemoryArena& arena,
                int depth = 0) const override;

private:
    // Private methods

    // Private fields
    int  nGathering_;
    bool cacheMode_;

    class IrradCache;
    const std::unique_ptr<IrradCache> cache_;

};

}  // namespace spica

#endif  // _SPICA_IRRADCACHE_H_
