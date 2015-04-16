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

#include "RendererBase.h"

namespace spica {
    
    class SPICA_BPT_RENDERER_DLL BPTRenderer : public RendererBase {

    public:
        explicit BPTRenderer(int width = 512, int height = 512, int samples = 4, int supsamples = 2);
        BPTRenderer(const BPTRenderer& renderer);
        ~BPTRenderer();

        BPTRenderer& operator=(const BPTRenderer& renderer);

        int render(const Scene& scene);

    private:
        void executeBPT(const Scene& scene, int x, int y);

        void executeLightTracing(const Scene& scene, int x, int y);
        void executePathTracing(const Scene& scene, int x, int y);
    };

}

#endif // SPICA_BPT_RENDERER_H_
