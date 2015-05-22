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

        /*
        struct HashGrid {
            int hashsize;
            BBox bbox;
            double hashScale;
            std::vector<HPoint> hps;
            std::vector<std::vector<HPoint*> > data;

            HashGrid()
                : hashsize(0)
                , bbox()
                , hashScale(0.0)
                , hps()
                , data()
            {
            }
        
            void construct(std::vector<HPoint>& hpoints, const int imageW, const int imageH) {
                for (int i = 0; i < hashsize; i++) {
                    data[i].clear();
                }

                const int numPoints = static_cast<int>(hpoints.size());

                // Compute bounding box
                Vector3 posMin(INFTY, INFTY, INFTY);
                Vector3 posMax(-INFTY, -INFTY, -INFTY);
                for (int i = 0; i < numPoints; i++) {
                    posMin = Vector3::minimum(posMin, static_cast<Vector3>(hpoints[i]));
                    posMax = Vector3::maximum(posMax, static_cast<Vector3>(hpoints[i]));
                }

                // Heuristic for initial radius
                Vector3 boxSize = posMax - posMin;
                const double irad = ((boxSize.x() + boxSize.y() + boxSize.z()) / 3.0) / ((imageW + imageH) / 2.0) * 2.0;

                // Update initial radius
                for (int i = 0; i < numPoints; i++) {
                    if (hpoints[i].n == 0) {
                        hpoints[i].r2 = irad * irad;
                        hpoints[i].n  = 0;
                        hpoints[i].flux = Color(0.0, 0.0, 0.0);
                    }

                    posMin = Vector3::minimum(posMin, Vector3(hpoints[i].x() - irad, hpoints[i].y() - irad, hpoints[i].z() - irad));
                    posMax = Vector3::maximum(posMax, Vector3(hpoints[i].x() + irad, hpoints[i].y() + irad, hpoints[i].z() + irad));
                }
                bbox = BBox(posMin, posMax);

                // Make each grid cell two times larger than the initial radius
                hashScale = 1.0 / (irad * 2.0);
                hashsize = numPoints;
                data.resize(hashsize);

                // Copy original data
                hps.resize(numPoints);
                std::copy(hpoints.begin(), hpoints.end(), hps.begin());

                // Set hit points to the grid
                for (int i = 0; i < numPoints; i++) {
                    HPoint& hp = hps[i];
                    Vector3 bMin = (Vector3(hp.x() - irad, hp.y() - irad, hp.z() - irad) - posMin) * hashScale;
                    Vector3 bMax = (Vector3(hp.x() + irad, hp.y() + irad, hp.z() + irad) - posMin) * hashScale;

                    const int minZ = std::abs(static_cast<int>(bMin.z()));
                    const int maxZ = std::abs(static_cast<int>(bMax.z()));
                    const int minY = std::abs(static_cast<int>(bMin.y()));
                    const int maxY = std::abs(static_cast<int>(bMax.y()));
                    const int minX = std::abs(static_cast<int>(bMin.x()));
                    const int maxX = std::abs(static_cast<int>(bMax.x()));
                    for (int iz = minZ; iz <= maxZ; iz++) {
                        for (int iy = minY; iy <= maxY; iy++) {
                            for (int ix = minX; ix <= maxX; ix++) {
                                int h = hash(ix, iy, iz);
                                data[h].push_back(&hp);
                            }
                        }
                    }
                }
            }

            unsigned int hash(const int ix, const int iy, const int iz) {
                return (unsigned int)((ix * 73856093) ^ (iy * 19349663) ^ (iz * 83492791)) % hashsize;   
            }

            const std::vector<HPoint*>& operator()(const Vector3 v) {
                Vector3 b = (v - bbox.posMin()) * hashScale;
                const int ix = std::abs(static_cast<int>(b.x()));
                const int iy = std::abs(static_cast<int>(b.y()));
                const int iz = std::abs(static_cast<int>(b.z()));
                return data[hash(ix, iy, iz)];
            }
        };
        */

        HashGrid<HPoint*> hashgrid;
        static const double ALPHA;

    public:
        PPMRenderer();
        PPMRenderer(const PPMRenderer& renderer);
        ~PPMRenderer();

        int render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel, const int numPhotons);

    private:
        // 1st pass: Trace rays from camera
        void traceRays(const Scene& scene, const Camera& camera, const Random& rng, std::vector<HPoint>& hpoints);

        // 2nd pass: Trace photons from lights
        void tracePhotons(const Scene& scene, const Random& rng, const int numPhotons);

        void executePathTracing(const Scene& scene, const Camera& camera, const Random& rng, HPoint* hp, const int bounceLimit = 6);

        void constructHashGrid(std::vector<HPoint>& hpoints, const int imageW, const int imageH);
    };

}

#endif  // _SPICA_PROGRESSIVE_PHOTONMAP_H_
