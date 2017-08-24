#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_GDPT_H_
#define _SPICA_GDPT_H_

#include <memory>

#include "core/core.hpp"
#include "core/common.h"
#include "core/integrator.h"

namespace spica {

class SPICA_EXPORTS GDPTIntegrator : public Integrator {
public:
    GDPTIntegrator(const std::shared_ptr<Sampler> &sampler);
    GDPTIntegrator(RenderParams &params);

    void render(const std::shared_ptr<const Camera> &camera,
                const Scene &scene,
                RenderParams &params) override;

    std::shared_ptr<Sampler> sampler_;
};

SPICA_EXPORT_PLUGIN(GDPTIntegrator, "Gradient domain path tracing");

}  // namespace spica

#endif  // _SPICA_GDPT_H_
