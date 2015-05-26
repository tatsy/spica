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

#include "scene.h"
#include "camera.h"
#include "../utils/uncopyable.h"
#include "../utils/kdtree.h"
#include "../utils/hash_grid.h"

namespace spica {

    class SPICA_PROGRESSIVE_PHOTONMAP_DLL PPMRenderer : public Uncopyable {
    private:

        struct HPoint : public Vector3 {
            Vector3 normal;
            Color flux;
            Color weight;
            Color emission;
            double coeff;
            int imageX, imageY;
            double r2;
            int n;

            HPoint(Vector3 pos = Vector3())
                : Vector3(pos)
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
                : Vector3()
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
                Vector3::operator=(hp);
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

            void setPosition(const Vector3& p) {
                this->_x = p.x();
                this->_y = p.y();
                this->_z = p.z();
            }
        };

        HashGrid<HPoint*> hashgrid;
        static const double ALPHA;

    public:
        PPMRenderer();
        PPMRenderer(const PPMRenderer& renderer);
        ~PPMRenderer();

        int render(const Scene& scene, const Camera& camera, Random& rng, const int samplePerPixel, const int numPhotons);

    private:
        // 1st pass: Trace rays from camera
        void traceRays(const Scene& scene, const Camera& camera, Random& rng, std::vector<HPoint>& hpoints);

        // 2nd pass: Trace photons from lights
        void tracePhotons(const Scene& scene, Random& rng, const int numPhotons);

        void executePathTracing(const Scene& scene, const Camera& camera, Random& rng, HPoint* hp, const int bounceLimit = 6);

        void constructHashGrid(std::vector<HPoint>& hpoints, const int imageW, const int imageH);
    };

}

#endif  // _SPICA_PROGRESSIVE_PHOTONMAP_H_
