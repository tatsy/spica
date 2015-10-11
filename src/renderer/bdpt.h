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

#include "../core/forward_decl.h"
#include "renderer_interface.h"

namespace spica {
    
    /** Bidirectional path tracing
     *  @ingroup renderer_module
     */
    class SPICA_BPT_RENDERER_DLL BDPTRenderer : public IRenderer {
    public:
        BDPTRenderer(spica::Image* image = NULL);
        ~BDPTRenderer();

        void render(const Scene& scene, const Camera& camera, const RenderParameters& params) override;
    };

}

#endif // SPICA_BPT_RENDERER_H_
