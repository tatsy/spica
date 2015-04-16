#ifndef SPICA_RENDERER_H_
#define SPICA_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_RENDERER_EXPORT
        #define SPICA_RENDERER_DLL __declspec(dllexport)
    #else
        #define SPICA_RENDERER_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_RENDERER_DLL
#endif

#include <string>

#include "RendererBase.h"
#include "Ray.h"
#include "Scene.h"
#include "../utils/Random.h"
#include "material.h"


namespace spica {

    class SPICA_RENDERER_DLL Renderer : public RendererBase {

    public:
        explicit Renderer(int width = 512, int height = 512, int sample = 4, int supsample = 2);
        Renderer(const Renderer& renderer);
        ~Renderer();

        Renderer& operator=(const Renderer& renderer);

        void setSize(int width, int height);
        void setSamples(int sample, int supsample);

        int render(const Scene& scene);

    private:
        static Color radiance(const Scene& scene, const Ray& ray, Random& rng, const int depth);
        static void savePPM(std::string filename, Color* image, int width, int height);
    };

}

#endif  // SPICA_RENDERER_H_
