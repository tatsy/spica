#ifndef _SPICA_SSS_RENDERER_H_
#define _SPICA_SSS_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SSS_RENDERER_EXPORT
        #define SPICA_SSS_RENDERER_DLL __declspec(dllexport)
    #else
        #define SPICA_SSS_RENDERER_DLL __declspec(dllexport)
    #endif
#else
    #define SPICA_SSS_RENDERER_DLL
#endif

#include "scene.h"
#include "camera.h"

#include "../utils/uncopyable.h"

namespace spica {

    // --------------------------------------------------
    // Subsurface scattering renderer
    // --------------------------------------------------
    class SPICA_SSS_RENDERER_DLL SSSRenderer : public Uncopyable {
    private:
        struct IrradiancePoint {
            Vector3 pos;
            Vector3 normal;
            double area;
            Color irad;

            IrradiancePoint()
                : pos()
                , normal()
                , area(0.0)
                , irad()
            {
            }

            IrradiancePoint(const Vector3& pos_, const Vector3& normal_, const double area_, const Color& irad_)
                : pos(pos_)
                , normal(normal_)
                , area(area_)
                , irad(irad_)
            {
            }

            IrradiancePoint(const IrradiancePoint& p)
                : pos()
                , normal()
                , area(0.0)
                , irad()
            {
                this->operator=(p);
            }

            IrradiancePoint& operator=(const IrradiancePoint& p) {
                this->pos = p.pos;
                this->normal = p.normal;
                this->area = p.area;
                this->irad = p.irad;
                return *this;
            }
        };

        struct OctreeNode {
            IrradiancePoint pt;
            BBox bbox;
            OctreeNode* children[8];
            bool isLeaf;

            OctreeNode()
                : pt()
                , bbox()
                , isLeaf(false)
            {
                for (int i = 0; i < 8; i++) {
                    children[i] = NULL;
                }
            }
                
        };

        class Octree {
        private:
            OctreeNode* _root;
            int* _numCopies;

        public:
            Octree();
            ~Octree();

            Octree(const Octree& octree);
            Octree& operator=(const Octree& octree);

            void construct(std::vector<IrradiancePoint>& iradPoints);

            Color iradSubsurface(const Vector3& pos, const DiffusionReflectance& Rd);

        private:
            void release();
            void deleteNode(OctreeNode* node);
            OctreeNode* constructRec(std::vector<IrradiancePoint>& iradPoints, const BBox& bbox);

            Color iradSubsurfaceRec(OctreeNode* node, const Vector3& pos, const DiffusionReflectance& Rd);
        };

        Octree octree;

    public:
        SSSRenderer();
        ~SSSRenderer();

        void render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel, const int samplerPerPoint);

    private:
        Color executePathTracing(const Scene& scene, const Camera& camera, const Random& rng, const int imageX, const int imageY);

        Color radiance(const Scene& scene, const Ray& ray, const Random& rng, const int depth, const int depthLimit = 6, const int maxDepth = 64);
    };
}

#endif  // _SPICA_SSS_RENDERER_H_
