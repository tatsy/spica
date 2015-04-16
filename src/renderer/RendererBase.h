#ifndef SPICA_RENDERER_BASE_H_
#define SPICA_RENDERER_BASE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_RENDERER_BASE_EXPORT
        #define SPICA_RENDERER_BASE_DLL __declspec(dllexport)
    #else
        #define SPICA_RENDERER_BASE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_RENDERER_BASE_DLL
#endif

#include "../utils/Random.h"
#include "Scene.h"

namespace spica {

    class SPICA_RENDERER_BASE_DLL RendererBase {
    protected:
        static const Color  backgroundColor;
        static const int    maxDepth;
        static const int    depthLimit;
        static const Random rng;

        int _width;
        int _height;
        int _samplePerPixel;
        int _supsamplePerAxis;

    public:
        explicit RendererBase(int width = 512, int height = 512, int samples = 4, int supsamples = 2);
        RendererBase(const RendererBase& renderer);
        virtual ~RendererBase();

        RendererBase& operator=(const RendererBase& renderer);
        
        virtual int render(const Scene& scene) = 0;
    };

}

#endif  // SPICA_RENDERER_BASE_H_
