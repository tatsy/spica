#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_PT_RENDERER_H_
#define SPICA_PT_RENDERER_H_

#include <string>

#include "../core/forward_decl.h"
#include "renderer_interface.h"

namespace spica {

    /** Unidirectional path tracing
     *  @ingroup renderer_module
     */
    class SPICA_EXPORTS PathRenderer : public IRenderer {
    public:
        /** The path tracing renderer constructor.
         */
        PathRenderer();

        /** The path tracing renderer destructor.
         */
        ~PathRenderer();

        /** Rendering process.
         *  @param scene: The redering scene.
         *  @param camera: The camera for the scene.
         *  @param filem: The film which stores the resulting image.
         *  @param params: The rendering parameters.
         */
        void render(const Scene& scene, const Camera& camera,
                    const std::unique_ptr<Film>& film,
                    const RenderParameters& params) override;

    private:
        Spectrum tracePath(const Scene& scene, const Camera& camera,
                           const RenderParameters& params,
                           const double pixelX, const double pixelY,
                           Sampler& sampler);

        Spectrum Li(const Scene& scene,
                    const RenderParameters& params,
                    const Ray& ray,
                    Sampler& sampler,
                    int bounces) const;
    };
}

#endif  // SPICA_PT_RENDERER_H_
