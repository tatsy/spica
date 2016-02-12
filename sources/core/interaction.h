#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_INTERACTION_H_
#define _SPICA_INTERACTION_H_

#include <memory>

#include "../core/common.h"
#include "../core/spectrum.h"
#include "../core/point2d.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"
#include "../math/vector3d.h"
#include "../bsdf/brdf.h"

namespace spica {

    class SPICA_EXPORTS Interaction {
    public:
        Interaction();
        explicit Interaction(const Point& p, const Normal& n = Normal(),
                             const Vector3D& dir = Vector3D());
        Interaction(const Interaction& intr);

        virtual ~Interaction();

        Interaction& operator=(const Interaction& intr);

        virtual Ray nextRay(const Vector3D& wi) const;

        inline const Point& pos() const { return pos_; }
        inline const Normal& normal() const { return normal_; }
        inline const Vector3D& dir() const { return dir_; }
        inline const Point2D& uv() const { return uv_; }
        inline double dudx() const { return dudx_; }
        inline double dudy() const { return dudy_; }
        inline double dvdx() const { return dvdx_; }
        inline double dvdy() const { return dvdy_; }

    protected:
        Point    pos_;
        Normal   normal_;
        Vector3D dir_;
        Point2D  uv_;
        double dudx_, dudy_, dvdx_, dvdy_;
    };

    class SPICA_EXPORTS SurfaceInteraction : public Interaction {
    public:
        SurfaceInteraction();
        SurfaceInteraction(const Point& p, const Normal& n, const Vector3D& dir,
                           const Point2D& uv);
        SurfaceInteraction(const SurfaceInteraction& intr);
        virtual ~SurfaceInteraction();

        SurfaceInteraction& operator=(const SurfaceInteraction& intr);

        Spectrum Le(const Vector3D& w) const;

        inline std::unique_ptr<AbstractBSDF>& bsdf() { return bsdf_; }
    
    private:
        Point2D uv_;
        std::unique_ptr<AbstractBSDF> bsdf_;
    };

}  // namespace spica

#endif  // _SPICA_INTERACTION_H_
