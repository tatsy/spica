#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PSSMLT_H_
#define _SPICA_PSSMLT_H_

#include "core/common.h"
#include "core/cobject.h"
#include "core/integrator.h"

namespace spica {

// Forward declaration
class PathSample;

/**
 * Primary Sample Space Metropolis Light Transport.
 * @ingroup integrator_module
 */
class SPICA_EXPORTS PSSMLTIntegrator : public Integrator {
public:
    // Public methods
    PSSMLTIntegrator();
    explicit PSSMLTIntegrator(RenderParams &params);
    virtual void render(const std::shared_ptr<const Camera>& camera, const Scene& scene, RenderParams& params) override;

private:
    PathSample generateSample(const std::shared_ptr<const Camera> &camera,
                              const Scene& scene,
                              RenderParams& params,
                              Sampler& sampler,
                              MemoryArena& arena) const;

    Spectrum Li(const Scene& scene,
                RenderParams& params,
                const Ray& r,
                Sampler& sampler,
                MemoryArena& arena,
                int depth = 0) const;
};

SPICA_EXPORT_PLUGIN(PSSMLTIntegrator, "Primary sampling space Metroplis light transport");

}  // namespace spica

#endif  // _SPICA_PSSMLT_H_
