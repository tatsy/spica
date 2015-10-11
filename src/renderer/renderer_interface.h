/**
 *  @defgroup renderer_module Renderers
 *  @brief Renderer implementations
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _RENDERER_INTERFACE_H_
#define _RENDERER_INTERFACE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_RENDERER_INTERFACE_EXPORT
        #define SPICA_RENDERER_INTERFACE_DLL __declspec(dllexport)
    #else
        #define SPICA_RENDERER_INTERFACE_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_RENDERER_INTERFACE_DLL
#endif

#include "../core/image.h"
#include "../core/uncopyable.h"
#include "../core/forward_decl.h"

namespace spica {

    /** Interface for renderer implementations
     *  @ingroup renderer_module
     */
    class SPICA_RENDERER_INTERFACE_DLL IRenderer : private Uncopyable {
        
    protected:
        Image _result;
        SubsurfaceIntegrator* _integrator;

    public:
        IRenderer();
        virtual ~IRenderer();

        virtual void render(const Scene& scene, const Camera& camera, 
                            const RenderParameters& params) = 0;
    };

}  // namespace spica

#endif  // _RENDERER_INTERFACE_H_
