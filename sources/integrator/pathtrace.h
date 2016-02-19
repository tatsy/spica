#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_PT_RENDERER_H_
#define SPICA_PT_RENDERER_H_

#include <string>

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "integrator.h"

namespace spica {

    /** 
     * Unidirectional path tracing
     * @ingroup renderer_module
     */
    class SPICA_EXPORTS PathIntegrator : public Integrator {
    public:
        // Public methods
        PathIntegrator(std::shared_ptr<Camera>& camera,
                     std::shared_ptr<Sampler>& sampler);
        ~PathIntegrator();
        void render(const Scene& scene,
                    const RenderParameters& params) const override;

    private:
        // Private methods
        Spectrum Li(const Scene& scene,
                    const RenderParameters& params,
                    const Ray& ray,
                    Sampler& sampler,
                    MemoryArena& arena,
                    int depth = 0) const;

        // Private fields
        std::shared_ptr<Sampler> sampler_;
    };
}

#endif  // SPICA_PT_RENDERER_H_
