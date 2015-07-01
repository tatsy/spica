#ifndef _SPICA_SUBSURFACE_INTEGRATOR_H_
#define _SPICA_SUBSURFACE_INTEGRATOR_H_

#include "renderer_constants.h"
#include "photon_mapping.h"
#include "bssrdf.h"

namespace spica {

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

    class SubsurfaceIntegrator : private Uncopyable {
    private:
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
            SubsurfaceIntegrator* _parent;

        public:
            Octree();
            ~Octree();

            Octree(const Octree& octree);
            Octree& operator=(const Octree& octree);

            void construct(SubsurfaceIntegrator* parent, std::vector<IrradiancePoint>& ipoints);

            Color iradSubsurface(const Vector3& pos, const BSSRDF& Rd) const;

        private:
            void release();
            void deleteNode(OctreeNode* node);
            OctreeNode* constructRec(std::vector<IrradiancePoint>& pointers, const BBox& bbox);

            Color iradSubsurfaceRec(OctreeNode* node, const Vector3& pos, const BSSRDF& Rd) const;
        };


    private:
        Material mtrl;
        BSSRDF bssrdf;
        PhotonMap photonMap;
        Octree octree;
        double dA;
        double _maxError;

    public:
        SubsurfaceIntegrator();
        ~SubsurfaceIntegrator();

        void initialize(const Scene& scene, const BSSRDF& bssrdf, const PMParams& params, const double areaRadius, const RandomType randType, const double maxError = 0.05);

        void buildOctree(const std::vector<Vector3>& points, const std::vector<Vector3>& normals, const PMParams& params);

        Color irradiance(const Vector3& p) const;

    private:
        void buildPhotonMap(const Scene& scene, const int numPhotons, const int bounceLimit, const RandomType randType);

        Color irradianceWithPM(const Vector3& p, const Vector3& n, const PMParams& params) const;
    };

}

#endif  // _SPICA_SUBSURFACE_INTEGRATOR_H_
