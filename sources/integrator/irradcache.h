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

class SPICA_EXPORTS IrradCacheIntegrator : public SamplerIntegrator {
public:
    IrradCacheIntegrator(const std::shared_ptr<const Camera>& camera,
                         const std::shared_ptr<Sampler>& sampler);
    ~IrradCacheIntegrator();

    void initialize(const Scene& scene,
                    const RenderParameters& params,
                    Sampler& sampler) override;

    void loopStarted(const Scene& scene,
                     const RenderParameters& params,
                     Sampler& sampler) override;

    void loopFinished(const Scene& scene,
                      const RenderParameters& params,
                      Sampler& sampler) override;

    Spectrum Li(const Scene& scene,
                const RenderParameters& params,
                const Ray& ray,
                Sampler& sampler,
                MemoryArena& arena,
                int depth = 0) const override;

private:
    

    int  nGathering_;
    bool cacheMode_;

    class IrradCache;
    const std::unique_ptr<IrradCache> cache_;

};

}  // namespace spica

#endif  // _SPICA_IRRADCACHE_H_
