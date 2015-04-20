#ifndef SPICA_MLT_RENDERER_H_
#define SPICA_MLT_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_MLT_RENDERER_EXPORT
        #define SPICA_MLT_RENDERER_DLL __declspec(dllexport)
    #else
        #define SPICA_MLT_RENDERER_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_MLT_RENDERER_DLL
#endif

#include "camera.h"
#include "scene.h"
#include "renderer_base.h"

namespace spica {

    class SPICA_MLT_RENDERER_DLL MLTRenderer {
    public:
        MLTRenderer();
        ~MLTRenderer();

        int render(const Scene& scene, const Camera& camera);
    }

}  // namespace spica

#endif  // SPICA_MLT_RENDERER_H_
