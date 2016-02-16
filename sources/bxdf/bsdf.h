#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BSDF_H_
#define _SPICA_BSDF_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "../math/vector3d.h"
#include "../core/normal3d.h"
#include "bxdf.h"

namespace spica {

class BSDF {
public:
    // Public methods
    BSDF(const SurfaceInteraction& isect, double eta = 1.0);

    void add(BxDF* b);
    int numComponents(BxDFType type = BxDFType::All) const;
    
    Spectrum f(const Vector3D& wiWorld, const Vector3D& woWorld,
               BxDFType type = BxDFType::All) const;

    Spectrum sample(const Vector3D& woWorld, Vector3D* wiWorld, const Point2D& rands,
                    double* pdf, BxDFType type = BxDFType::All) const;
    double pdf(const Vector3D& wo, const Vector3D& wi,
               BxDFType type = BxDFType::All) const;

    bool hasType(BxDFType type) const;

    inline double eta() const { return eta_; }

private:
    // Private methods
    Vector3D worldToLocal(const Vector3D& v) const;
    Vector3D localToWorld(const Vector3D& v) const;

    // Private fields
    const double eta_;
    const Normal3D normal_;
    const Vector3D tangent_, binormal_;
    int nBxDFs_ = 0;
    static constexpr int maxBxDFs_ = 8;
    BxDF *bxdfs_[maxBxDFs_];

};  // class BSDF

}  // namespace spica

#endif  // _SPICA_BSDF_H_
