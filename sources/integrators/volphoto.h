#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VOLPHOTO_H_
#define _SPICA_VOLPHOTO_H_

#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "integrator.h"

namespace spica {

/** Volumetric photon mapping
 *  @ingroup renderer_module
 */
class SPICA_EXPORTS VolPhotoIntegrator : public SamplerIntegrator {
public:
    VolPhotoIntegrator(const std::shared_ptr<const Camera>& camera,
                       const std::shared_ptr<Sampler>& sampler,
                       double alpha = 0.8);

    ~VolPhotoIntegrator();

    void initialize(const Scene& scene,
                    const RenderParams& params,
                    Sampler& sampler) override;

    void loopStarted(const Scene& scene,
                     const RenderParams& params,
                     Sampler& sampler) override;

    void loopFinished(const Scene& scene,
                      const RenderParams& parmas,
                      Sampler& sampler) override;

    Spectrum Li(const Scene& scene,
                const RenderParams& params,
                const Ray& ray,
                Sampler& sampler,
                MemoryArena& arena,
                int depth = 0) const override;

private:
    // Private fields
    class VPhotonMap;
    
    std::unique_ptr<VPhotonMap> photonmap_;
    double    globalRadius_;
    const double alpha_;
};

}  // namespace spica

#endif  // _SPICA_VOLPHOTO_H_
