#ifndef _SPICA_SUBSURFACE_INTEGRATOR_H_
#define _SPICA_SUBSURFACE_INTEGRATOR_H_

#include "renderer_constants.h"
#include "photon_map.h"
#include "bssrdf.h"

namespace spica {

    struct HitpointInfo : public Vector3 {
        Vector3 normal;
        Color flux;
        Color weight;
        Color emission;
        int imageX, imageY;
        double coeff;
        double r2;
        int n;
        double area;
        bool isPixel;

        HitpointInfo(const Vector3& pos = Vector3(), const Vector3& normal_ = Vector3())
            : Vector3(pos)
            , normal(normal_)
            , flux()
            , weight()
            , emission()
            , imageX(-1)
            , imageY(-1)
            , coeff(0.0)
            , r2(0.0)
            , n(0)
            , area(0.0)
            , isPixel(true)
        {
        }

        void setPosition(const Vector3& p) {
            this->_x = p.x();
            this->_y = p.y();
            this->_z = p.z();
        }
    };

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

    class SubsurfaceIntegrator : public Uncopyable {
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

            void construct(SubsurfaceIntegrator* parent, std::vector<IrradiancePoint>& hpoints);

            Color iradSubsurface(const Vector3& pos, const DiffusionReflectance& Rd) const;

        private:
            void release();
            void deleteNode(OctreeNode* node);
            OctreeNode* constructRec(std::vector<IrradiancePoint>& pointers, const BBox& bbox);

            Color iradSubsurfaceRec(OctreeNode* node, const Vector3& pos, const DiffusionReflectance& Rd) const;
        };


    private:
        Material mtrl;
        BSSRDF bssrdf;
        Octree octree;

    public:
        SubsurfaceIntegrator();
        ~SubsurfaceIntegrator();

        void init(const Scene& scene, const Camera& camera, const BSSRDF& bssrdf, const double areaRadius, std::vector<HitpointInfo>* hpoints);

        void buildOctree(const std::vector<HitpointInfo>& hpoints, const int numShotPhoton, const double areaRadius);

        Color irradiance(const Vector3& p) const;
    };

}

#endif  // _SPICA_SUBSURFACE_INTEGRATOR_H_
