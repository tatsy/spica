#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BDPT_INTEGRATOR_H_
#define _SPICA_BDPT_INTEGRATOR_H_

#include "core/common.h"
#include "core/renderparams.h"
#include "core/integrator.h"

namespace spica {
    
/** Bidirectional path tracing
 *  @ingroup renderer_module
 */
class SPICA_EXPORTS BDPTIntegrator : public Integrator {
public:
    // Public methods
    explicit BDPTIntegrator(const std::shared_ptr<Sampler>& sampler);
    explicit BDPTIntegrator(RenderParams &params);
    ~BDPTIntegrator();

    void render(const std::shared_ptr<const Camera>& camera,
                const Scene& scene,
                RenderParams& params) override;

private:
    std::shared_ptr<Sampler> sampler_ = nullptr;
};

SPICA_EXPORT_PLUGIN(BDPTIntegrator, "Bidirectional path tracing integrator");

}  // namespace spica

#endif // _SPICA_BDPT_INTEGRATOR_H_
