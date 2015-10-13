#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_PT_RENDERER_H_
#define SPICA_PT_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_PT_RENDERER_EXPORT
        #define SPICA_PT_RENDERER_DLL __declspec(dllexport)
    #else
        #define SPICA_PT_RENDERER_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_PT_RENDERER_DLL
#endif

#include <string>

#include "../core/forward_decl.h"
#include "renderer_interface.h"

namespace spica {

    /** Unidirectional path tracing
     *  @ingroup renderer_module
     */
    class SPICA_PT_RENDERER_DLL PathRenderer : public IRenderer {
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
         *  @param params: The rendering parameters.
         */
        void render(const Scene& scene, const Camera& camera,
                    const RenderParameters& params) override;

    private:
        Color tracePath(const Scene& scene, const Camera& camera,
                        const RenderParameters& params,
                        const double pixelX, const double pixelY,
                        Stack<double>& rands);

        Color radiance(const Scene& scene,
                       const RenderParameters& params,
                       const Ray& ray,
                       Stack<double>& rstack,
                       int bounces) const;

        Color directSample(const Scene& scene, const int triID,
                           const Vector3D& in, const Vector3D& v,
                           const Vector3D& n, const Color& refl,
                           int bounces, Stack<double>& rands) const;
    };
}

#endif  // SPICA_PT_RENDERER_H_
