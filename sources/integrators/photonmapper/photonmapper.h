#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PHOTON_MAPPER_H_
#define _SPICA_PHOTON_MAPPER_H_

#include "core/common.h"
#include "core/integrator.h"
#include "core/renderparams.h"
#include "integrators/photon_map.h"

namespace spica {

/** Progressive photon mapping: a probabilistic approach
 *  @ingroup renderer_module
 */
class SPICA_EXPORTS PhotonMapperIntegrator : public SamplerIntegrator {
public:
    explicit PhotonMapperIntegrator(const std::shared_ptr<Sampler> &sampler, double alpha = 0.8);
    explicit PhotonMapperIntegrator(RenderParams &params);

    void initialize(const std::shared_ptr<const Camera> &camera,
                    const Scene& scene,
                    RenderParams& params,
                    Sampler& sampler) override;

    void loopStarted(const std::shared_ptr<const Camera> &camera,
                     const Scene& scene,
                     RenderParams& params,
                     Sampler& sampler) override;

    void loopFinished(const std::shared_ptr<const Camera> &camera,
                      const Scene& scene,
                      RenderParams& params,
                      Sampler& sampler) override;

    Spectrum Li(const Scene& scene,
                RenderParams& params,
                const Ray& r,
                Sampler& sampler,
                MemoryArena& arena,
                int depth) const override;

private:
    // Private fields
    PhotonMap photonmap_;
    double globalRadius_;
    const double alpha_;
};

SPICA_EXPORT_PLUGIN(PhotonMapperIntegrator, "Photon mapping (PPM: a probabilistic approach)")

}  // namespace spica

#endif  // _SPICA_PHOTON_MAPPER_H_
