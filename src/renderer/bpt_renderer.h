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

#include "renderer_constants.h"
#include "renderer_helper.h"

namespace spica {
    
    class SPICA_BPT_RENDERER_DLL BDPTRenderer : public Uncopyable {
    private:
        spica::Image* _image;

    public:
        BDPTRenderer(spica::Image* image = NULL);
        ~BDPTRenderer();

        void render(const Scene& scene, const Camera& camera, const int samplePerPixel, const RandomType randType);
    };

}

#endif // SPICA_BPT_RENDERER_H_
