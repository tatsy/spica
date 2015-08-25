#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SUBSURFACE_INTEGRATOR_H_
#define _SPICA_SUBSURFACE_INTEGRATOR_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SUBSURFACE_INTEGRATOR_EXPORT
        #define SPICA_SUBSURFACE_INTEGRATOR_DLL __declspec(dllexport)
    #else
        #define SPICA_SUBSURFACE_INTEGRATOR_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_SUBSURFACE_INTEGRATOR_DLL
#endif

#include "renderer_constants.h"
#include "ppmprob.h"
#include "bssrdf.h"

namespace spica {

    struct IrradiancePoint {
        Vector3D pos;
        Vector3D normal;
        double area;
        Color irad;

        IrradiancePoint()
            : pos()
            , normal()
            , area(0.0)
            , irad()
        {
        }

        IrradiancePoint(const Vector3D& pos_, const Vector3D& normal_,
                        const double area_, const Color& irad_)
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

    /*! Irradiance integrator for subsurface scattering objects
     */
    class SPICA_SUBSURFACE_INTEGRATOR_DLL SubsurfaceIntegrator
        : private Uncopyable {
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

            void construct(SubsurfaceIntegrator* parent,
                           std::vector<IrradiancePoint>& ipoints);

            Color iradSubsurface(const Vector3D& pos, const BSSRDF& Rd) const;

        private:
            void release();
            void deleteNode(OctreeNode* node);
            OctreeNode* constructRec(std::vector<IrradiancePoint>& pointers,
                                     const BBox& bbox);

            Color iradSubsurfaceRec(OctreeNode* node, const Vector3D& pos,
                                    const BSSRDF& Rd) const;
        };

    private:
        PhotonMap _photonMap;
        Octree    _octree;
        double    _dA;
        double    _maxError;

    public:
        SubsurfaceIntegrator();
        ~SubsurfaceIntegrator();

        void initialize(const Scene& scene, const RenderParameters& params,
                        const double maxError = 0.05);

        void buildOctree(const std::vector<Vector3D>& points,
                         const std::vector<Vector3D>& normals,
                         const RenderParameters& params);

        Color irradiance(const Vector3D& p, const BSDF& bsdf) const;
    };

}  // namespace spica

#endif  // _SPICA_SUBSURFACE_INTEGRATOR_H_
