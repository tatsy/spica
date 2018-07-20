#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MMLT_H_
#define _SPICA_MMLT_H_

#include "core/common.h"
#include "core/integrator.h"

namespace spica {

// Forward declaration
class PathSample;
class PSSSampler;

/**
 * Multiplexed Metropolis Light Transport.
 * @ingroup integrator_module
 */
class SPICA_EXPORTS MMLTIntegrator : public Integrator {
public:
    // Public methods
    MMLTIntegrator();
    explicit MMLTIntegrator(RenderParams &params);
    virtual void render(const std::shared_ptr<const Camera>& camera, const Scene& scene, RenderParams& params) override;

private:
    // Private methods
    PathSample generateSample(const std::shared_ptr<const Camera> &camera,
                              const Scene& scene,
                              RenderParams& params,
                              PSSSampler& sampler,
                              MemoryArena& arena,
                              const Distribution1D &lightDist,
                              int depth) const;

    Spectrum Li(const std::shared_ptr<const Camera> &camera,
                const Scene& scene, PSSSampler& sampler, MemoryArena& arena,
                int depth, const Distribution1D &lightDist,
                Point2d *pixel) const;
};

SPICA_EXPORT_PLUGIN(MMLTIntegrator, "Multiplexed Metropolis light transport");

}  // namespace spica

#endif  // _SPICA_MMLT_H_
