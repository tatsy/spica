#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_DIRECTLIGHTING_H_
#define _SPICA_DIRECTLIGHTING_H_

#include "core/integrator.h"

namespace spica {

class SPICA_EXPORTS DirectLightingIntegrator : public SamplerIntegrator {
public:
    // Public methods
    explicit DirectLightingIntegrator(const std::shared_ptr<Sampler>& sampler);
    explicit DirectLightingIntegrator(RenderParams &params);

    Spectrum Li(const Scene& scene,
                RenderParams& params,
                const Ray& ray,
                Sampler& sampler,
                MemoryArena& arena,
                int depth = 0) const override;
};

SPICA_EXPORT_PLUGIN(DirectLightingIntegrator, "Integrate only direct lighting");

}  // namespace spica

#endif  // _SPICA_DIRECTLIGHTING_H_
