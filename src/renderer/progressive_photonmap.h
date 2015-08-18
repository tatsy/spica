#ifndef _SPICA_PROGRESSIVE_PHOTONMAP_H_
#define _SPICA_PROGRESSIVE_PHOTONMAP_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_PROGRESSIVE_PHOTONMAP_EXPORT
        #define SPICA_PROGRESSIVE_PHOTONMAP_DLL __declspec(dllexport)
    #else
        #define SPICA_PROGRESSIVE_PHOTONMAP_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_PROGRESSIVE_PHOTONMAP_DLL
#endif

#include "renderer_constants.h"
#include "../utils/uncopyable.h"
#include "../utils/kdtree.h"
#include "../utils/hash_grid.h"

namespace spica {

    class SPICA_PROGRESSIVE_PHOTONMAP_DLL PPMRenderer : private Uncopyable {
    private:

        struct HPoint : public Vector3D {
            Vector3D normal;
            Color flux;
            Color weight;
            Color emission;
            double coeff;
            int imageX, imageY;
            double r2;
            int n;

            explicit HPoint(Vector3D pos = Vector3D())
                : Vector3D(pos)
                , normal()
                , flux()
                , weight()
                , emission()
                , coeff(0.0)
                , imageX(-1)
                , imageY(-1)
                , r2(0.0)
                , n(0)
            {
            }

            HPoint(const HPoint& hp)
                : Vector3D()
                , normal()
                , flux()
                , weight()
                , emission()
                , coeff(0.0)
                , imageX(-1)
                , imageY(-1)
                , r2(0.0)
                , n(0)
            {
                operator=(hp);
            }

            HPoint& operator=(const HPoint& hp) {
                Vector3D::operator=(hp);
                this->normal = hp.normal;
                this->flux = hp.flux;
                this->weight = hp.weight;
                this->emission = hp.emission;
                this->coeff = hp.coeff;
                this->imageX = hp.imageX;
                this->imageY = hp.imageY;
                this->r2 = hp.r2;
                this->n = hp.n;
                return *this;
            }

            void setPosition(const Vector3D& p) {
                this->x() = p.x();
                this->y() = p.y();
                this->z() = p.z();
            }
        };

        HashGrid<HPoint*> hashgrid;
        static const double ALPHA;

    public:
        PPMRenderer();
        ~PPMRenderer();

        void render(const Scene& scene, const Camera& camera, const int samplePerPixel, const int numPhotons, const RandomType randType = PSEUDO_RANDOM_TWISTER);

    private:
        // 1st pass: Trace rays from camera
        void traceRays(const Scene& scene, const Camera& camera, RandomBase* rand, std::vector<HPoint>& hpoints);

        // 2nd pass: Trace photons from lights
        void tracePhotons(const Scene& scene, RandomBase* rand, const int numPhotons);

        void executePathTracing(const Scene& scene, const Camera& camera, Stack<double>& rseq, HPoint* hp, const int bounceLimit = 64);

        void constructHashGrid(std::vector<HPoint>& hpoints, const int imageW, const int imageH);
    };

}

#endif  // _SPICA_PROGRESSIVE_PHOTONMAP_H_
