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

#include "ray.h"
#include "scene.h"
#include "camera.h"
#include "material.h"
#include "../random/random.h"

namespace spica {

    class SPICA_PT_RENDERER_DLL PTRenderer {

    public:
        PTRenderer();
        PTRenderer(const PTRenderer& renderer);
        ~PTRenderer();

        PTRenderer& operator=(const PTRenderer& renderer);

        int render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel);

    private:

        static Color executePT(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, const Random& rng);
    };
}

#endif  // SPICA_PT_RENDERER_H_
