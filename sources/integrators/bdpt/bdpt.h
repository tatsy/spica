#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BDPT_INTEGRATOR_H_
#define _SPICA_BDPT_INTEGRATOR_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "integrator.h"

namespace spica {
    
/** Bidirectional path tracing
 *  @ingroup renderer_module
 */
class SPICA_EXPORTS BDPTIntegrator : public Integrator {
public:
    // Public methods
    BDPTIntegrator(const std::shared_ptr<const Camera>& camera,
                   const std::shared_ptr<Sampler>& sampler);
    ~BDPTIntegrator();

    void render(const Scene& scene,
                const RenderParams& params) override;

private:
    std::shared_ptr<Sampler> sampler_ = nullptr;
};

}  // namespace spica

#endif // _SPICA_BDPT_INTEGRATOR_H_
