#ifndef _SPICA_SUBSURFACE_SPPM_RENDERER_H_
#define _SPICA_SUBSURFACE_SPPM_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SUBSURFACE_SPPM_RENDERER_EXPORT
        #define SPICA_SUBSURFACE_SPPM_RENDERER_DLL __declspec(dllexport)
    #else
        #define SPICA_SUBSURFACE_SPPM_RENDERER_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_SUBSURFACE_SPPM_DLL
#endif

#include "subsurface_integrator.h"
#include "renderer_constants.h"
#include "../utils/hash_grid.h"

namespace spica {

    class SPICA_SUBSURFACE_SPPM_RENDERER_DLL SubsurfaceSPPMRenderer : public Uncopyable {
    private:
        SubsurfaceIntegrator integrator;
        HashGrid<HitpointInfo*> hashgrid;
        static const double ALPHA;

    public:
        SubsurfaceSPPMRenderer();
        ~SubsurfaceSPPMRenderer();

        void render(const Scene& scene, const Camera& camera, const int samplePerPixel, const int numPhotons, const RandomType randType = PSEUDO_RANDOM_TWISTER);

    private:
        // 1st pass: Trace rays from camera
        void traceRays(const Scene& scene, const Camera& camera, RandomBase* rand, std::vector<HitpointInfo>& hpoints);

        // 2nd pass: Trace photons from lights
        void tracePhotons(const Scene& scene, RandomBase* rand, const int numPhotons, const int bounceLimit = 64);

        void executePathTracing(const Scene& scene, const Camera& camera, RandomSeq& rseq, HitpointInfo* hp, const int bounceLimit = 64);

        void constructHashGrid(std::vector<HitpointInfo>& hpoints, const int imageW, const int imageH);


    };

}

#endif  // _SPICA_SUBSURFACE_SPPM_RENDERER_H_
