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
    PathSample calcPathSample() const;
    Spectrum Li(const Scene& scene,
                const RenderParams& params,
                const Ray& r,
                Sampler& sampler,
                MemoryArena& arena,
                int depth) const;

    class PSSSampler;
    std::shared_ptr<Sampler> sampler_ = nullptr;
};

}  // namespace spica

#endif  // _SPICA_PSSMLT_H_
