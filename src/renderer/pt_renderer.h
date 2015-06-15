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

#include "renderer_constants.h"
#include "../utils/uncopyable.h"
#include "../random/random.h"
#include "../random/halton.h"

namespace spica {

    // --------------------------------------------------
    // Monte Carlo path tracing renderer
    // --------------------------------------------------
    class SPICA_PT_RENDERER_DLL PathTracingRenderer : public Uncopyable {

    public:
        PathTracingRenderer();
        ~PathTracingRenderer();

        void render(const Scene& scene, const Camera& camera, const int samplePerPixel, RandomType randType = PSEUDO_RANDOM_TWISTER);
        
    private:
        static Color executePathTracing(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, RandomSeq& rseq);
    };
}

#endif  // SPICA_PT_RENDERER_H_
