#ifndef _SPICA_SUBSURFACE_SPPM_RENDERER_H_
#define _SPICA_SUBSURFACE_SPPM_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SUBSURFACE_SPPM_RENDERER_EXPORT
        #define SPICA_SUBSURFACE_SPPM_RENDERER_DLL __declspec(dllexport)
    #else
        #define SPICA_SUBSURFACE_SPPM_RENDERER_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_SUBSURFACE_SPPM_RENDERER_DLL
#endif

#include "subsurface_integrator.h"
#include "renderer_constants.h"
#include "../utils/hash_grid.h"

namespace spica {

    struct HitpointInfo : public Vector3D {
        Vector3D normal;
        Color flux;
        Color weight;
        Color emission;
        double coeff;
        double r2;
        double area;
        int imageX;
        int imageY;
        int n;
        bool isPixel;

        explicit HitpointInfo(const Vector3D& pos = Vector3D())
            : Vector3D(pos)
            , normal()
            , flux()
            , weight()
            , emission()
            , coeff(0.0)
            , r2(0.0)
            , area(0.0)
            , imageX(-1)
            , imageY(-1)
            , n(0)
            , isPixel(true)
        {
        }

        HitpointInfo(const HitpointInfo& hp)
            : Vector3D()
            , normal()
            , flux()
            , weight()
            , emission()
            , coeff(0.0)
            , r2(0.0)
            , area(0.0)
            , imageX(-1)
            , imageY(-1)
            , n(0)
            , isPixel(true)
        {
            operator=(hp);
        }

        HitpointInfo& operator=(const HitpointInfo& hp) {
            Vector3D::operator=(hp);
            this->normal = hp.normal;
            this->flux = hp.flux;
            this->weight = hp.weight;
            this->emission = hp.emission;
            this->coeff = hp.coeff;
            this->r2 = hp.r2;
            this->area = hp.area;
            this->imageX = hp.imageX;
            this->imageY = hp.imageY;
            this->n = hp.n;
            this->isPixel = hp.isPixel;
            return *this;
        }

        void setPosition(const Vector3D& p) {
            this->x() = p.x();
            this->y() = p.y();
            this->z() = p.z();
        }
    };

    class SPICA_SUBSURFACE_SPPM_RENDERER_DLL SubsurfaceSPPMRenderer : public Uncopyable {
    private:
        Image* _image;
        SubsurfaceIntegrator integrator;
        HashGrid<HitpointInfo*> hashgrid;
        static const double ALPHA;

    public:
        explicit SubsurfaceSPPMRenderer(Image* image = NULL);
        ~SubsurfaceSPPMRenderer();

        void render(const Scene& scene, const Camera& camera, const BSSRDF& bssrdf, const int samplePerPixel, const int numPhotons, const RandomType randType = PSEUDO_RANDOM_TWISTER);

    private:
        // 1st pass: Trace rays from camera
        void traceRays(const Scene& scene, const Camera& camera, RandomBase* rand, std::vector<HitpointInfo>& hpoints);

        // 2nd pass: Trace photons from lights
        void tracePhotons(const Scene& scene, RandomBase* rand, const int numPhotons, const int bounceLimit = 64);

        void executePathTracing(const Scene& scene, const Camera& camera, Stack<double>& rseq, HitpointInfo* hp, const int bounceLimit = 64);

        void constructHashGrid(std::vector<HitpointInfo>& hpoints, const int imageW, const int imageH);


    };

}

#endif  // _SPICA_SUBSURFACE_SPPM_RENDERER_H_
