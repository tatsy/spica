#ifndef _SPICA_PROGRESSIVE_PHOTONMAP_H_
#define _SPICA_PROGRESSIVE_PHOTONMAP_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifndef SPICA_PROGRESSIVE_PHOTONMAP_EXPORT
        #define SPICA_PROGRESSIVE_PHOTONMAP_DLL __declspec(dllexport)
    #else
        #define SPICA_PROGRESSIVE_PHOTONMAP_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_PROGRESSIVE_PHOTONMAP_DLL
#endif

#include "scene.h"
#include "camera.h"

namespace spica {

    class SPICA_PROGRESSIVE_PHOTONMAP_DLL PPMRenderer {
    private:

    public:
        PPMRenderer();
        PPMRenderer(const PPMRenderer& renderer);
        ~PPMRenderer();

        int render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel) const;

    private:
        // 1st pass: Trace rays from camera
        void traceRays(const Scene& scene, const Camera& camera, const Random& rng);

        // 2nd pass: Trace photons from lights
        void tracePhotons(const Scene& scene, const Random& rng, const int numPhotons);

        void executePathTracing(const Scene& scene, const Camera& camera, const Random& rng, const int imageX, const int imageY);
    };

}

#endif  // _SPICA_PROGRESSIVE_PHOTONMAP_H_
