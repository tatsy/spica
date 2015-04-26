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

namespace spica {

    class SPICA_MLT_RENDERER_DLL MLTRenderer {
    public:
        MLTRenderer();
        ~MLTRenderer();

		int render(const Scene& scene, const int mlt_num, const int mutation, Image& image, const Ray& camera, const Vector3& cx, const Vector3& cy, const int width, const int height, const int maxDepth, const Random& rng);
	};

}  // namespace spica

#endif  // SPICA_MLT_RENDERER_H_
