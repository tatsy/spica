#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_INTERACTION_H_
#define _SPICA_INTERACTION_H_

#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "../core/spectrum.h"
#include "../core/point2d.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"
#include "../math/vector3d.h"

namespace spica {

    class SPICA_EXPORTS Interaction {
    public:
        Interaction();
        explicit Interaction(const Point& pos, const Normal& normal = Normal(),
                             const Vector3D& wo = Vector3D());
        Interaction(const Interaction& intr);
        virtual ~Interaction();

        Interaction& operator=(const Interaction& intr);

        virtual Ray nextRay(const Vector3D& wi) const;

        inline virtual bool isSurfaceInteraction() const { return false; }
        inline const Point&    pos()    const { return pos_; }
        inline const Normal&   normal() const { return normal_; }
        inline const Vector3D& wo()     const { return wo_; }

    protected:
        Point    pos_;
        Normal   normal_;
        Vector3D wo_;
    };

    class SPICA_EXPORTS SurfaceInteraction : public Interaction {
    public:
        SurfaceInteraction();
        SurfaceInteraction(const Point& p, const Point2D& uv, const Vector3D& wo,
                           const Vector3D& dpdu, const Vector3D& dpdv,
                           const Normal3D& dndu, const Normal3D& dndv,
                           const Shape* shape);
        SurfaceInteraction(const SurfaceInteraction& intr);
        virtual ~SurfaceInteraction();

        SurfaceInteraction& operator=(const SurfaceInteraction& intr);

        void computeDifferentials(const Ray& ray);
        void setScatterFuncs(const Ray& ray, MemoryArena& arena);
        Spectrum Le(const Vector3D& w) const;

        inline bool isSurfaceInteraction() const override { return true; }
        inline const Point2D& uv() const { return uv_; }
        inline const Vector3D& dpdu() const { return dpdu_; }
        inline const Vector3D& dpdv() const { return dpdv_; }
        inline const Normal3D& dndu() const { return dndu_; }
        inline const Normal3D& dndv() const { return dndv_; }
        inline double dudx() const { return dudx_; }
        inline double dudy() const { return dudy_; }
        inline double dvdx() const { return dvdx_; }
        inline double dvdy() const { return dvdy_; }
        inline const Primitive* primitive() const { return primitive_; }

        inline BSDF* bsdf() const { return bsdf_; }
        inline void setBSDF(BSDF* bsdf) { bsdf_ = bsdf; }
        inline void setPrimitive(const Primitive* prim) { primitive_ = prim; }
    
    private:
        Point2D uv_;
        Vector3D dpdu_, dpdv_;
        Normal3D dndu_, dndv_;
        Vector3D dpdx_, dpdy_;
        double dudx_ = 0.0, dudy_ = 0.0, dvdx_ = 0.0, dvdy_ = 0.0;
        const Shape* shape_ = nullptr;
        const Primitive* primitive_ = nullptr;
        BSDF* bsdf_ = nullptr;
    };

}  // namespace spica

#endif  // _SPICA_INTERACTION_H_
