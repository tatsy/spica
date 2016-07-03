#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PSSMLT_H_
#define _SPICA_PSSMLT_H_

#include "integrator.h"

namespace spica {

/**
 * Primary Sample Space Metropolis Light Transport.
 * @ingroup integrator_module
 */
class PSSMLTIntegrator : public Integrator {
public:
    // Public methods
    explicit PSSMLTIntegrator(const std::shared_ptr<const Camera>& camera);
    virtual ~PSSMLTIntegrator();
    virtual void render(const Scene& scene, const RenderParams& params) override;

private:
    class PathSample;
    PathSample generateSample(const Scene& scene,
                              const RenderParams& params,
                              Sampler& sampler,
                              MemoryArena& arena) const;

    Spectrum Li(const Scene& scene,
                const RenderParams& params,
                const Ray& r,
                Sampler& sampler,
                MemoryArena& arena,
                int depth = 0) const;
};

}  // namespace spica

#endif  // _SPICA_PSSMLT_H_
