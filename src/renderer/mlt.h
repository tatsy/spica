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

#include "renderer_interface.h"
#include "renderer_constants.h"

namespace spica {

    class SPICA_MLT_RENDERER_DLL MLTRenderer : private IRenderer {
    private:
        spica::Image* _image;

    public:
        explicit MLTRenderer(spica::Image* image = NULL);
        ~MLTRenderer();

        // ------------------------------------------------------------
        // MLT rendering process
        // ------------------------------------------------------------
        // @param[in] scene: scene to be rendered
        // @param[in] camera: camera viewing the scene
        // @param[in] rng: random number generator
        // @param[in] numMLT: # of independent MLT process to be executed
        // @param[in] numMutate: # of mutation in a single MLT process
        // @param[in] maxDepth: maximum depth for path tracing recursion
        void render(const Scene& scene, const Camera& camera, Random& rng, int numMLT, int numMutate, int maxDepth);
    };

}  // namespace spica

#endif  // SPICA_MLT_RENDERER_H_
