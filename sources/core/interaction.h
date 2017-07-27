#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_INTERACTION_H_
#define _SPICA_INTERACTION_H_

#include <memory>

#include "core/core.hpp"
#include "core/common.h"
#include "core/spectrum.h"
#include "core/point2d.h"
#include "core/point3d.h"
#include "core/normal3d.h"
#include "core/vector3d.h"

#include "core/render.hpp"
#include "core/medium.h"

namespace spica {

class SPICA_EXPORTS Interaction {
public:
    Interaction();
    explicit Interaction(const Point3d& pos, const Normal3d& normal = Normal3d(),
                         const Vector3d& wo = Vector3d());
    Interaction(const Point3d& pos, const Vector3d& wo,
                const MediumInterface& mediumInterface);
    Interaction(const Interaction& intr);
    virtual ~Interaction();

    Interaction& operator=(const Interaction& intr);

    virtual Ray spawnRay(const Vector3d& wi) const;
    virtual Ray spawnRayTo(const Point3d& p) const;
    virtual Ray spawnRayTo(const Interaction& intr) const;

    inline virtual bool isSurfaceInteraction() const { return false; }
    inline const Point3d&    pos()    const { return pos_; }
    inline const Normal3d&   normal() const { return normal_; }
    inline const Vector3d& wo()     const { return wo_; }
    inline void setMediumInterface(const MediumInterface& mediumInterface) {
        mediumInterface_ = mediumInterface;
    }

    inline const Medium* getMedium(const Vector3d& w) const {
        return vect::dot(w, normal_) > 0.0 ? mediumInterface_.outside()
                                           : mediumInterface_.inside(); 
    }

    inline const Medium* getMedium() const {
        Assertion(mediumInterface_.inside() == mediumInterface_.outside(),
                  "Only inside the primitive should be fiiled with participating media");
        return mediumInterface_.inside();
    }

protected:
    Point3d  pos_;
    Normal3d normal_;
    Vector3d wo_;
    MediumInterface mediumInterface_;

private:
    friend class SeparableBSSRDF;
};

class SPICA_EXPORTS SurfaceInteraction : public Interaction {
public:
    SurfaceInteraction();
    SurfaceInteraction(const Point3d& p, const Point2d& uv, const Vector3d& wo,
                       const Vector3d& dpdu, const Vector3d& dpdv,
                       const Normal3d& dndu, const Normal3d& dndv,
                       const Shape* shape);
    SurfaceInteraction(const SurfaceInteraction& intr);
    virtual ~SurfaceInteraction();

    SurfaceInteraction& operator=(const SurfaceInteraction& intr);

    void computeDifferentials(const Ray& ray);
    void setScatterFuncs(const Ray& ray, MemoryArena& arena);
    Spectrum Le(const Vector3d& w) const;
    
    inline bool isSurfaceInteraction() const override { return true; }
    inline const Point2d& uv() const { return uv_; }
    inline const Vector3d& dpdu() const { return dpdu_; }
    inline const Vector3d& dpdv() const { return dpdv_; }
    inline const Normal3d& dndu() const { return dndu_; }
    inline const Normal3d& dndv() const { return dndv_; }
    inline double dudx() const { return dudx_; }
    inline double dudy() const { return dudy_; }
    inline double dvdx() const { return dvdx_; }
    inline double dvdy() const { return dvdy_; }
    inline const Primitive* primitive() const { return primitive_; }

    inline BSDF* bsdf() const { return bsdf_; }
    inline BSSRDF* bssrdf() const { return bssrdf_; }
    inline void setBSDF(BSDF* bsdf) { bsdf_ = bsdf; }
    inline void setBSSRDF(BSSRDF* bssrdf) { bssrdf_ = bssrdf; }
    inline void setPrimitive(const Primitive* prim) { primitive_ = prim; }
    
private:
    Point2d uv_;
    Vector3d dpdu_, dpdv_;
    Normal3d dndu_, dndv_;
    Vector3d dpdx_, dpdy_;
    double dudx_ = 0.0, dudy_ = 0.0, dvdx_ = 0.0, dvdy_ = 0.0;
    const Shape* shape_ = nullptr;
    const Primitive* primitive_ = nullptr;
    BSDF* bsdf_ = nullptr;
    BSSRDF* bssrdf_ = nullptr;
};

class SPICA_EXPORTS MediumInteraction : public Interaction {
public:
    MediumInteraction();
    MediumInteraction(const Point3d& p, const Vector3d& wo,
                      const Medium* medium,
                      const PhaseFunction* phase);
    bool isValid() const;

    inline const PhaseFunction* phase() const { return phase_; }

private:
    const PhaseFunction* phase_;
};

}  // namespace spica

#endif  // _SPICA_INTERACTION_H_
