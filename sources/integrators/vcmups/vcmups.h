#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VCMUPS_H_
#define _SPICA_VCMUPS_H_

#include "core/common.h"
#include "core/renderparams.h"
#include "core/integrator.h"

namespace spica {

class SPICA_EXPORTS VCMUPSIntegrator : public Integrator {
public:
    explicit VCMUPSIntegrator(const std::shared_ptr<Sampler> &sampler, double alpha = 0.8);
    explicit VCMUPSIntegrator(RenderParams &params);

    void render(const std::shared_ptr<const Camera> &camera,
                const Scene &scene,
                RenderParams &params) override;

    void initialize(const std::shared_ptr<const Camera> &camera,
                    const Scene& scene,
                    RenderParams& params,
                    Sampler& sampler);

    void loopStarted(const std::shared_ptr<const Camera> &camera,
                     const Scene& scene,
                     RenderParams& params,
                     Sampler& sampler);

    void loopFinished(const std::shared_ptr<const Camera> &camera,
                      const Scene& scene,
                      RenderParams& params,
                      Sampler& sampler);

private:
    std::shared_ptr<Sampler> sampler_ = nullptr;
    double alpha_;
    double lookupRadiusScale_ = 1.0;
};

SPICA_EXPORT_PLUGIN(VCMUPSIntegrator, "Vertex connection and merging (VCM) / Unified path sampling (UPS)");

}  // namespace spica

#endif  // _SPICA_VCMUPS_H_
