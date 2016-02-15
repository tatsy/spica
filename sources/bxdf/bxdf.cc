#define SPICA_API_EXPORT
#include "bxdf.h"

#include "../core/sampling.h"
#include "../math/vector3d.h"

namespace spica {

namespace {

inline double absCosTheta(const Vector3D& w) { return std::abs(w.z()); }

}  // anonymous namespace

// -----------------------------------------------------------------------------
// BxDF
// -----------------------------------------------------------------------------

BxDF::BxDF(BxDFType type)
    : type_{ type } {
}

BxDF::~BxDF() {
}

Spectrum BxDF::sample(const Vector3D& wo, Vector3D* wi, const Point2D& rands,
                      double* pdf) const {
    *wi = sampleCosineHemisphere(rands);
    if (wo.z() < 0.0) wi->zRef() *= -1.0;
    *pdf = this->pdf(wo, *wi);
    return f(wo, *wi);
}

double BxDF::pdf(const Vector3D& wo, const Vector3D& wi) const {
    return wo.z() * wi.z() > 0.0 ? absCosTheta(wi) * INV_PI : 0.0;
}


// -----------------------------------------------------------------------------
// LambertianReflection
// -----------------------------------------------------------------------------

LambertianReflection::LambertianReflection()
    : BxDF{ BxDFType::Reflection | BxDFType::Diffuse }
    , ref_{} {
}

LambertianReflection::LambertianReflection(const Spectrum& ref)
    : LambertianReflection{} {
    this->ref_ = ref;
}

Spectrum LambertianReflection::f(const Vector3D& wo, const Vector3D& wi) const {
    return ref_ * INV_PI;
}


// -----------------------------------------------------------------------------
// SpecularReflection
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// FresnelSpecular
// -----------------------------------------------------------------------------


}  // namespace spica