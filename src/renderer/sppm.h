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

#include "renderer_interface.h"
#include "renderer_constants.h"
#include "../utils/kdtree.h"
#include "../utils/hash_grid.h"

namespace spica {

    class SPICA_SPPM_DLL SPPMRenderer : private IRenderer {
    private:

        struct SPPMPixel {
            Vector3D position;
            Vector3D normal;
            Color flux;
            Color weight;
            Color emission;
            double coeff;
            int x, y;
            double r2;
            int n;

            explicit SPPMPixel(Vector3D pos = Vector3D())
                : position(pos)
                , normal()
                , flux()
                , weight()
                , emission()
                , coeff(0.0)
                , x(-1)
                , y(-1)
                , r2(0.0)
                , n(0)
            {
            }

            SPPMPixel(const SPPMPixel& pixel)
                : position()
                , normal()
                , flux()
                , weight()
                , emission()
                , coeff(0.0)
                , x(-1)
                , y(-1)
                , r2(0.0)
                , n(0)
            {
                operator=(pixel);
            }

            SPPMPixel& operator=(const SPPMPixel& pixel) {
                this->position = pixel.position;
                this->normal   = pixel.normal;
                this->flux     = pixel.flux;
                this->weight   = pixel.weight;
                this->emission = pixel.emission;
                this->coeff    = pixel.coeff;
                this->x        = pixel.x;
                this->y        = pixel.y;
                this->r2       = pixel.r2;
                this->n        = pixel.n;
                return *this;
            }

            Color radiance() const {
                const Color  rad = Color(flux / (PI * r2));
                return Color((emission + rad) * coeff);
            }
        };

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

}

#endif  // _SPICA_SPPM_H_
