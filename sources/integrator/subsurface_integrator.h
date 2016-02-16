#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SUBSURFACE_INTEGRATOR_H_
#define _SPICA_SUBSURFACE_INTEGRATOR_H_

#include "../core/forward_decl.h"
#include "../core/bound3d.h"
#include "photon_map.h"

namespace spica {

    struct IrradiancePoint {
        Point pos;
        Normal normal;
        double area;
        Spectrum irad;

        IrradiancePoint()
            : pos()
            , normal()
            , area(0.0)
            , irad()
        {
        }

        IrradiancePoint(const Point& pos_, const Normal& normal_,
                        const double area_, const Spectrum& irad_)
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

    /** Irradiance integrator for subsurface scattering objects
     *  @ingroup renderer_module
     */
    class SPICA_EXPORTS SubsurfaceIntegrator : private Uncopyable {
    private:
        struct OctreeNode {
            IrradiancePoint pt;
            Bound3d bbox;
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

            void construct(SubsurfaceIntegrator* parent,
                           std::vector<IrradiancePoint>& ipoints);

            Spectrum iradSubsurface(const Point& pos, const BSSRDF& Rd) const;

        private:
            void release();
            void deleteNode(OctreeNode* node);
            OctreeNode* constructRec(std::vector<IrradiancePoint>& pointers,
                                     const Bound3d& bbox);

            Spectrum iradSubsurfaceRec(OctreeNode* node, const Point& pos,
                                    const BSSRDF& Rd) const;
        };

    private:
        PhotonMap _photonMap;
        Octree    _octree;
        double    _dA;
        double    _radius;
        double    _maxError;
        std::vector<Triangle> _triangles;

    public:
        SubsurfaceIntegrator();
        ~SubsurfaceIntegrator();

        void initialize(const Scene& scene,
                        const double maxError = 0.05);

        void construct(const Scene& scene,
                       const RenderParameters& params);

        Spectrum irradiance(const Point& p, const BSDF& bsdf) const;

    private:
        void buildOctree(const std::vector<Point>& points,
                         const std::vector<Normal>& normals,
                         const RenderParameters& params);
    };

}  // namespace spica

#endif  // _SPICA_SUBSURFACE_INTEGRATOR_H_