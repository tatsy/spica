#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MLT_RENDERER_H_
#define _SPICA_MLT_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_MLT_RENDERER_EXPORT
        #define SPICA_MLT_RENDERER_DLL __declspec(dllexport)
    #else
        #define SPICA_MLT_RENDERER_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_MLT_RENDERER_DLL
#endif

#include "../core/forward_decl.h"
#include "renderer_interface.h"

namespace spica {

    /** Metropolis light transport.
     *  @ingroup renderer_module
     */
    class SPICA_MLT_RENDERER_DLL MLTRenderer : private IRenderer {
    public:
        /** Constructor.
         */
        MLTRenderer();

        /** Destructor.
         */
        ~MLTRenderer();

        /** MLT rendering process.
         *  @param scene: The rendering scene
         *  @param camera: The camera viewing the scene
         *  @param params: Rendering parameters
         */
        void render(const Scene& scene, const Camera& camera, const RenderParameters& params) override;
    };

}  // namespace spica

#endif  // SPICA_MLT_RENDERER_H_
