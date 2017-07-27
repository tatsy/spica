#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BSDF_H_
#define _SPICA_BSDF_H_

#include <array>

#include "core/core.hpp"
#include "core/common.h"
#include "core/vector3d.h"
#include "core/normal3d.h"

#include "core/render.hpp"
#include "core/bxdf.h"

namespace spica {

class SPICA_EXPORTS BSDF {
public:
    // Public methods
    BSDF(const SurfaceInteraction& isect, double eta = 1.0);

    void add(BxDF* b);
    int numComponents(BxDFType type = BxDFType::All) const;
    
    Spectrum f(const Vector3d& wiWorld, const Vector3d& woWorld,
               BxDFType type = BxDFType::All) const;

    Spectrum sample(const Vector3d& woWorld, Vector3d* wiWorld, const Point2d& rands,
                    double* pdf, BxDFType type = BxDFType::All,
                    BxDFType* sampledType = nullptr) const;
    double pdf(const Vector3d& wo, const Vector3d& wi,
               BxDFType type = BxDFType::All) const;

    bool hasType(BxDFType type) const;

    inline double eta() const { return eta_; }

private:
    // Private methods
    Vector3d worldToLocal(const Vector3d& v) const;
    Vector3d localToWorld(const Vector3d& v) const;

    // Private fields
    const double eta_;
    const Normal3d normal_;
    const Vector3d tangent_, binormal_;
    int nBxDFs_ = 0;
    static constexpr int maxBxDFs_ = 8;
    std::array<BxDF*, maxBxDFs_> bxdfs_;

};  // class BSDF

}  // namespace spica

#endif  // _SPICA_BSDF_H_
