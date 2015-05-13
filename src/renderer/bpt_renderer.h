#ifndef SPICA_BPT_RENDERER_H_
#define SPICA_BPT_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_BPT_RENDERER_EXPORT
        #define SPICA_BPT_RENDERER_DLL __declspec(dllexport)
    #else
        #define SPICA_BPT_RENDERER_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_BPT_RENDERER_DLL
#endif

#include "scene.h"
#include "camera.h"
#include "renderer_helper.h"

namespace spica {
    
    class SPICA_BPT_RENDERER_DLL BPTRenderer {

    public:
        BPTRenderer();
        BPTRenderer(const BPTRenderer& renderer);
        ~BPTRenderer();

        BPTRenderer& operator=(const BPTRenderer& renderer);

        int render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel);

        int renderPT(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel);

        int renderLT(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel);
    };

}

#endif // SPICA_BPT_RENDERER_H_
