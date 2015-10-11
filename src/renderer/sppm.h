#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SPPM_H_
#define _SPICA_SPPM_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SPPM_EXPORT
        #define SPICA_SPPM_DLL __declspec(dllexport)
    #else
        #define SPICA_SPPM_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_SPPM_DLL
#endif

#include "../core/forward_decl.h"
#include "renderer_interface.h"
#include "../core/hash_grid.h"

namespace spica {

    /** Stochastic progressive photon mapping
     *  @ingroup renderer_module
     */
    class SPICA_SPPM_DLL SPPMRenderer : private IRenderer {
    private:

        struct SPPMPixel;

        HashGrid<SPPMPixel*> hashgrid;
        static const double kAlpha;

    public:
        SPPMRenderer();
        ~SPPMRenderer();

        void render(const Scene& scene, const Camera& camera,
                    const RenderParameters& params);
        
    private:
        // 1st pass: Trace rays from camera
        void traceRays(const Scene& scene, const Camera& camera,
                       const RenderParameters& params,
                       RandomSampler* samplers,
                       std::vector<SPPMPixel>& hpoints);

        // 2nd pass: Trace photons from lights
        void tracePhotons(const Scene& scene,
                          const RenderParameters& params,
                          RandomSampler* samplers,
                          const int numPhotons) const;

        void tracePhotonsRec(const Scene& scene, const Ray& ray,
                             const RenderParameters& params,
                             const Color& flux,
                             int bounces,
                             Stack<double>& rstk) const;

        void pathTrace(const Scene& scene, const Camera& camera,
                       const RenderParameters& params,
                       Stack<double>& rseq, SPPMPixel* hp);

        void constructHashGrid(std::vector<SPPMPixel>& hpoints,
                               const int imageW, const int imageH);
    };

}  // namespace spica

#endif  // _SPICA_SPPM_H_
