#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VOLPHOTO_H_
#define _SPICA_VOLPHOTO_H_

#include <memory>

#include "core/common.h"
#include "core/integrator.h"

namespace spica {

/** Volumetric photon mapping
 *  @ingroup renderer_module
 */
class SPICA_EXPORTS VolPhotoIntegrator : public SamplerIntegrator {
public:
    explicit VolPhotoIntegrator(const std::shared_ptr<Sampler> &sampler, double alpha = 0.8);
    explicit VolPhotoIntegrator(RenderParams &params);

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
                const Ray& ray,
                Sampler& sampler,
                MemoryArena& arena,
                int depth) const override;

private:
    // Private fields
    class VPhotonMap;
    
    std::unique_ptr<VPhotonMap> photonmap_;
    double    globalRadius_;
    const double alpha_;
};

SPICA_EXPORT_PLUGIN(VolPhotoIntegrator, "Volumetric photon mapping (PPM: a probabilistic approach)")

}  // namespace spica

#endif  // _SPICA_VOLPHOTO_H_
