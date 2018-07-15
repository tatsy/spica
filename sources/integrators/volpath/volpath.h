#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VOLPATH_H_
#define _SPICA_VOLPATH_H_

#include <memory>

#include "core/core.hpp"
#include "core/common.h"
#include "core/integrator.h"
#include "core/renderparams.h"

namespace spica {

class SPICA_EXPORTS VolPathIntegrator : public SamplerIntegrator {
public:
    explicit VolPathIntegrator(const std::shared_ptr<Sampler> &sampler);
    explicit VolPathIntegrator(RenderParams &params);

    Spectrum Li(const Scene& scene,
                RenderParams& params,
                const Ray& r,
                Sampler& sampler,
                MemoryArena& arena,
                int depth) const override;
};

SPICA_EXPORT_PLUGIN(VolPathIntegrator, "Volumetric path tracing integrator");

}  // namespace spica


#endif  // _SPICA_VOLPATH_H_
