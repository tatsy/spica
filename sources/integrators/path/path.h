#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PATH_INTEGRATOR_H_
#define _SPICA_PATH_INTEGRATOR_H_

#include <string>

#include "core/common.h"
#include "core/core.hpp"

#include "core/integrator.h"

namespace spica {

/** 
 * Unidirectional path tracing
 * @ingroup renderer_module
 */
class SPICA_EXPORTS PathIntegrator : public SamplerIntegrator {
public:
    // Public methods
    explicit PathIntegrator(const std::shared_ptr<Sampler>& sampler);
    explicit PathIntegrator(RenderParams &params);
    ~PathIntegrator();

protected:
    // Protected methods
    virtual Spectrum Li(const Scene& scene,
                        RenderParams& params,
                        const Ray& ray,
                        Sampler& sampler,
                        MemoryArena& arena,
                        int depth = 0) const override;

    // Private
    std::shared_ptr<Sampler> sampler_;
};

SPICA_EXPORT_PLUGIN(PathIntegrator, "Path tracing integrator");

}  // namespace spica

#endif  // _SPICA_PATH_INTEGRATOR_H_
