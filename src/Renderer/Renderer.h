#ifndef RAINY_RENDERER_H_
#define RAINY_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef RAINY_RENDERER_EXPORT
        #define RAINY_RENDERER_DLL __declspec(dllexport)
    #else
        #define RAINY_RENDERER_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define RAINY_RENDERER_DLL
#endif

#include <string>

#include "Ray.h"
#include "Scene.h"
#include "../utils/Random.h"
#include "material.h"

namespace spica {

    class RAINY_RENDERER_DLL Renderer {
    private:
        int _width;              // Screen width
        int _height;             // Screen height
        int _samplePerPixel;     // # of samples / pixel
        int _supersamplePerAxis; // # of samples in pixel for an axis

        // --------------------
        // Constant
        // --------------------
        static const Color backgroundColor;
        static const int maxDepth;
        static const int depthLimit;

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

#endif  // RAINY_RENDERER_H_
