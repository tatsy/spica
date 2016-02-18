#define SPICA_API_EXPORT
#include "bxdf.h"

#include "../core/sampling.h"
#include "../math/vector3d.h"

namespace spica {

namespace {

inline double absCosTheta(const Vector3D& w) { return std::abs(w.z()); }

double FrDielectric(double cosThetaI, double etaI, double etaT) {
    cosThetaI = clamp(cosThetaI, -1.0, 1.0);

    bool entering = cosThetaI > 0.0;
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }

    double sinThetaI = sqrt(std::max(0.0, 1.0 - cosThetaI * cosThetaI));
    double sinThetaT = etaI / etaT * sinThetaI;

    // Total reflection
    if (sinThetaT >= 1.0) return 1.0;
    double cosThetaT = sqrt(std::max(0.0, 1.0 - sinThetaT * sinThetaT));
    double Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) / 
                   ((etaT * cosThetaI) + (etaI * cosThetaI));
    double Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) / 
                   ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2.0;
}

bool refract(const Vector3D& wi, const Vector3D& n, double eta, Vector3D* wt) {
    double cosThetaI = vect::dot(n, wi);
    double sin2ThetaI = std::max(0.0, 1.0 - cosThetaI * cosThetaI);
    double sin2ThetaT = eta * eta * sin2ThetaI;

    if (sin2ThetaI >= 1.0) return false;
    double cosThetaT = sqrt(1.0 - sin2ThetaT);
    *wt = eta * (-wi) + (eta * cosThetaI - cosThetaT) * Vector3D(n);
    return true;
}

template <class T>
inline Normal3_<T> Faceforward(const Normal3_<T>& n, const Vector3_<T>& v) {
    return vect::dot(n, v) < 0.0 ? -n : n;
}

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
                      double* pdf, BxDFType* type) const {
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

SpecularReflection::SpecularReflection()
    : BxDF{ BxDFType::Reflection | BxDFType::Specular }
    , ref_{} {
}

SpecularReflection::SpecularReflection(const Spectrum& ref)
    : SpecularReflection{} {
    this->ref_ = ref;
}

Spectrum SpecularReflection::f(const Vector3D& wo, const Vector3D& wi) const {
    return Spectrum(0.0);    
}

Spectrum SpecularReflection::sample(const Vector3D& wo, Vector3D* wi,
                                    const Point2D& rands, double* pdf,
                                    BxDFType* type) const {
    *wi = Vector3D(-wo.x(), -wo.y(), wo.z());
    *pdf = 1.0;
    // TODO: Fresnel reflection should be considered??
    return ref_ / std::abs(wo.z());
}

double SpecularReflection::pdf(const Vector3D& wo, const Vector3D& wi) const {
    return 0.0;   
}

// -----------------------------------------------------------------------------
// FresnelSpecular
// -----------------------------------------------------------------------------

FresnelSpecular::FresnelSpecular()
    : BxDF{ BxDFType::Reflection | BxDFType::Transmission | BxDFType::Specular }
    , ref_{}
    , tr_{} {
}

FresnelSpecular::FresnelSpecular(const Spectrum& ref, const Spectrum& tr,
                                 double etaA, double etaB)
    : FresnelSpecular{} {
    this->ref_  = ref;
    this->tr_   = tr;
    this->etaA_ = etaA;
    this->etaB_ = etaB;
}

Spectrum FresnelSpecular::f(const Vector3D& wo, const Vector3D& wi) const {
    return Spectrum(0.0);
}

Spectrum FresnelSpecular::sample(const Vector3D& wo, Vector3D* wi,
                                 const Point2D& rands, double* pdf,
                                 BxDFType* sampledType) const {
    const double refProb = FrDielectric(wo.z(), etaA_, etaB_);
    if (rands[0] < refProb) {
        // Reflection
        *wi = Vector3D(-wo.x(), -wo.y(), wo.z());
        if (sampledType) {
            *sampledType = BxDFType::Specular | BxDFType::Reflection;
        }
        *pdf = refProb;
        return ref_ * (refProb / std::abs(wi->z()));
    } else {
        // Transmission
        bool entering = wo.z() > 0.0;
        double etaI = entering ? etaA_ : etaB_;
        double etaT = entering ? etaB_ : etaA_;

        if (!refract(wo, Vector3D(Faceforward(Normal3D(0.0, 0.0, 1.0), wo)), etaI / etaT, wi)) {
            return Spectrum(0.0);
        }

        Spectrum ft = tr_ * (1.0 - refProb);
        ft *= (etaI * etaI) / (etaT * etaT);

        if (sampledType) {
            *sampledType = BxDFType::Specular | BxDFType::Transmission;
        }
        *pdf = 1.0 - refProb;
        return ft / std::abs(wi->z());
    }
}

double FresnelSpecular::pdf(const Vector3D& wo, const Vector3D& wi) const {
    return 0.0;
}

}  // namespace spica
