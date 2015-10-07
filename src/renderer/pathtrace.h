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
#elif defined(linux) || defined(__linux)
    #define SPICA_PT_RENDERER_DLL
#endif

#include <string>

#include "renderer_interface.h"
#include "renderer_constants.h"
#include "render_parameters.h"

#include "../random/random.h"
#include "../random/halton.h"

namespace spica {

    /*! Unidirectional path tracing
     */
    class SPICA_PT_RENDERER_DLL PathRenderer : public IRenderer {
    public:
        PathRenderer();
        ~PathRenderer();

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
                           const Vector3D& n, int bounces,
                           Stack<double>& rands) const;
    };
}

#endif  // SPICA_PT_RENDERER_H_
