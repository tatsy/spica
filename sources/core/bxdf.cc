#define SPICA_API_EXPORT
#include "bxdf.h"

#include "core/vector3d.h"
#include "core/normal3d.h"
#include "core/vect_math.h"

#include "core/sampling.h"
#include "core/fresnel.h"
#include "core/microfacet.h"

namespace spica {

namespace {

inline double absCosTheta(const Vector3d& w) { return std::abs(w.z()); }

static thread_local Random random;

}  // anonymous namespace

// -----------------------------------------------------------------------------
// BxDF
// -----------------------------------------------------------------------------

BxDF::BxDF(BxDFType type)
    : type_{ type } {
}

BxDF::~BxDF() {
}

Spectrum BxDF::sample(const Vector3d& wo, Vector3d* wi, const Point2d& rands,
                      double* pdf, BxDFType* type) const {
    *wi = sampleCosineHemisphere(rands);
    if (wo.z() < 0.0) wi->zRef() *= -1.0;
    *pdf = this->pdf(wo, *wi);
    return f(wo, *wi);
}

double BxDF::pdf(const Vector3d& wo, const Vector3d& wi) const {
    return wo.z() * wi.z() > 0.0 ? absCosTheta(wi) * INV_PI : 0.0;
}


// -----------------------------------------------------------------------------
// LambertianReflection
// -----------------------------------------------------------------------------

LambertianReflection::LambertianReflection(const Spectrum& ref)
    : BxDF{ BxDFType::Reflection | BxDFType::Diffuse } {
    this->ref_ = ref;
}

Spectrum LambertianReflection::f(const Vector3d& wo, const Vector3d& wi) const {
    return ref_ * INV_PI;
}


// -----------------------------------------------------------------------------
// LambertianTransmission
// -----------------------------------------------------------------------------

LambertianTransmission::LambertianTransmission(const Spectrum& tr)
    : BxDF{ BxDFType::Transmission | BxDFType::Diffuse } {
    this->tr_ = tr;
}

Spectrum LambertianTransmission::f(const Vector3d& wo,
                                   const Vector3d& wi) const {
    return tr_ * INV_PI;
}


// -----------------------------------------------------------------------------
// SpecularReflection
// -----------------------------------------------------------------------------

SpecularReflection::SpecularReflection(const Spectrum& ref, Fresnel* fresnel)
    : BxDF{ BxDFType::Reflection | BxDFType::Specular } {
    this->ref_ = ref;
    this->fresnel_ = fresnel;
}

Spectrum SpecularReflection::f(const Vector3d& wo, const Vector3d& wi) const {
    return Spectrum(0.0);    
}

Spectrum SpecularReflection::sample(const Vector3d& wo, Vector3d* wi,
                                    const Point2d& rands, double* pdf,
                                    BxDFType* type) const {
    *wi = Vector3d(-wo.x(), -wo.y(), wo.z());
    *pdf = 1.0;

    const double cosTheta = vect::cosTheta(*wi);
    return fresnel_->evaluate(cosTheta) * ref_ / std::abs(cosTheta);
}

double SpecularReflection::pdf(const Vector3d& wo, const Vector3d& wi) const {
    return 0.0;   
}


// -----------------------------------------------------------------------------
// SpecularReflection
// -----------------------------------------------------------------------------

SpecularTransmission::SpecularTransmission(const Spectrum& tr, double etaA,
                                           double etaB)
    : BxDF{ BxDFType::Transmission | BxDFType::Specular }
    , tr_{ tr }
    , etaA_{ etaA }
    , etaB_{ etaB }
    , fresnel_{ std::make_unique<FresnelDielectric>(etaA_, etaB_) } {
}

Spectrum SpecularTransmission::f(const Vector3d& wo, const Vector3d& wi) const {
    return Spectrum(0.0);    
}

Spectrum SpecularTransmission::sample(const Vector3d& wo, Vector3d* wi,
                                      const Point2d& rands, double* pdf,
                                      BxDFType* sampledType) const {
    bool entering = vect::cosTheta(wo) > 0.0;
    double etaI = entering ? etaA_ : etaB_;
    double etaT = entering ? etaB_ : etaA_;

    if (!vect::refract(wo, Vector3d(vect::faceforward(Normal3d(0.0, 0.0, 1.0), wo)),
        etaI / etaT, wi)) {
        return Spectrum(0.0);
    }

    *pdf = 1.0;
    Spectrum ft =
        tr_ * (Spectrum(1.0) - fresnel_->evaluate(vect::cosTheta(*wi)));

    return ft / std::abs(vect::cosTheta(*wi));
}

double SpecularTransmission::pdf(const Vector3d& wo, const Vector3d& wi) const {
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

Spectrum FresnelSpecular::f(const Vector3d& wo, const Vector3d& wi) const {
    return Spectrum(0.0);
}

Spectrum FresnelSpecular::sample(const Vector3d& wo, Vector3d* wi,
                                 const Point2d& rands, double* pdf,
                                 BxDFType* sampledType) const {
    const double refProb = FrDielectric(vect::cosTheta(wo), etaA_, etaB_);
    if (rands[0] < refProb) {
        // Reflection
        *wi = Vector3d(-wo.x(), -wo.y(), wo.z());
        if (sampledType) {
            *sampledType = BxDFType::Specular | BxDFType::Reflection;
        }
        *pdf = refProb;
        return ref_ * (refProb / std::abs(vect::cosTheta(*wi)));
    } else {
        // Transmission
        bool entering = vect::cosTheta(wo) > 0.0;
        double etaI = entering ? etaA_ : etaB_;
        double etaT = entering ? etaB_ : etaA_;

        if (!vect::refract(wo, Vector3d(vect::faceforward(Normal3d(0.0, 0.0, 1.0), wo)), etaI / etaT, wi)) {
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

double FresnelSpecular::pdf(const Vector3d& wo, const Vector3d& wi) const {
    return 0.0;
}


// -----------------------------------------------------------------------------
// MicrofacetReflection method definitions
// -----------------------------------------------------------------------------

MicrofacetReflection::MicrofacetReflection(const Spectrum& ref,
                                           MicrofacetDistribution* distrib,
                                           Fresnel* fresnel)
    : BxDF{ BxDFType::Reflection | BxDFType::Glossy }
    , ref_{ ref }
    , distrib_{ distrib }
    , fresnel_{ fresnel } {
}

Spectrum MicrofacetReflection::f(const Vector3d& wo, const Vector3d& wi) const {
    const double cosThetaO = std::abs(vect::cosTheta(wo));
    const double cosThetaI = std::abs(vect::cosTheta(wi));
    Vector3d wh = wi + wo;

    // Degenerate case
    if (cosThetaI == 0.0 || cosThetaO == 0.0) return Spectrum(0.0);
    if (wh.x() == 0.0 && wh.y() == 0.0 && wh.z() == 0.0) return Spectrum(0.0);

    wh = wh.normalized();
    Spectrum F = fresnel_->evaluate(vect::dot(wi, wh));
    auto ret = ref_ * distrib_->D(wh) * distrib_->G(wo, wi) * F /
               (4.0 * cosThetaI * cosThetaO);
    return ret;
}

Spectrum MicrofacetReflection::sample(const Vector3d& wo, Vector3d* wi,
                                      const Point2d& rands, double* pdf,
                                      BxDFType* sampledType) const {
    if (wo.z() == 0.0) return Spectrum(0.0);

    Vector3d wh = distrib_->sample(wo, rands);
    *wi = vect::reflect(wo, wh);
    if (!vect::sameHemisphere(wo, *wi)) return Spectrum(0.0);

    *pdf = distrib_->pdf(wo, wh) / (4.0 * vect::dot(wo, wh));
    return f(wo, *wi);
}

double MicrofacetReflection::pdf(const Vector3d& wo, const Vector3d& wi) const {
    if (!vect::sameHemisphere(wo, wi)) return 0.0;
    Vector3d wh = vect::normalize(wo + wi);
    return distrib_->pdf(wo, wh) / (4.0 * vect::dot(wo, wh));
}


// -----------------------------------------------------------------------------
// MicrofacetTransmission method definitions
// -----------------------------------------------------------------------------

MicrofacetTransmission::MicrofacetTransmission(const Spectrum &re, const Spectrum& tr,
                                               MicrofacetDistribution* distrib,
                                               double etaA, double etaB)
    : BxDF{ BxDFType::Reflection | BxDFType::Transmission | BxDFType::Glossy }
    , re_{ re }
    , tr_{ tr }
    , distrib_{ distrib }
    , etaA_{ etaA }
    , etaB_{ etaB } {
}

Spectrum MicrofacetTransmission::f(const Vector3d& wo,
                                   const Vector3d& wi) const {
    double cosThetaO = vect::cosTheta(wo);
    double cosThetaI = vect::cosTheta(wi);
    if (cosThetaO == 0.0 || cosThetaI == 0.0) return Spectrum(0.0);

    const double F = FrDielectric(cosThetaO, etaA_, etaB_);
    if (vect::sameHemisphere(wo, wi)) {
        // Reflection
        Vector3d wh = vect::normalize(wo + wi);
        return re_ * F * distrib_->D(wh) * distrib_->G(wo, wi) / (4.0 * cosThetaI * cosThetaO);
    } else {
        // Transmission
        double eta = cosThetaO > 0.0 ? (etaB_ / etaA_) : (etaA_ / etaB_);
        Vector3d wh = vect::normalize(wo + wi * eta);
        if (wh.z() < 0.0) wh = -wh;

        double sqrtDenom = vect::dot(wo, wh) + eta * vect::dot(wi, wh);
        return tr_ * (1.0 - F) * std::abs(distrib_->D(wh) * distrib_->G(wo, wi) * eta * eta *
               vect::absDot(wi, wh) * vect::absDot(wo, wh) /
               (cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));
    }
}

Spectrum MicrofacetTransmission::sample(const Vector3d& wo, Vector3d* wi,
                                        const Point2d& rands, double* pdf,
                                        BxDFType* sampledType) const {
    if (wo.z() == 0.0) return Spectrum(0.0);

    const double F = FrDielectric(vect::cosTheta(wo), etaA_, etaB_);
    if (random.get1D() < F) {
        // Reflection
        Vector3d wh = distrib_->sample(wo, rands);
        *wi = vect::reflect(wo, wh);
        if (sampledType) {
            *sampledType = BxDFType::Reflection | BxDFType::Glossy;
        }
    } else {
        // Transmission
        Vector3d wh = distrib_->sample(wo, rands);
        double eta = vect::cosTheta(wo) > 0.0 ? (etaA_ / etaB_) : (etaB_ / etaA_);
        if (!vect::refract(wo, wh, eta, wi)) return Spectrum(0.0);

        if (sampledType) {
            *sampledType = BxDFType::Transmission | BxDFType::Glossy;
        }
    }

    *pdf = this->pdf(wo, *wi);
    return f(wo, *wi);
}

double MicrofacetTransmission::pdf(const Vector3d& wo,
                                   const Vector3d& wi) const {
    const double F = FrDielectric(vect::cosTheta(wo), etaA_, etaB_);
    if (vect::sameHemisphere(wo, wi)) {
        // Reflection
        Vector3d wh = vect::normalize(wi + wo);
        return distrib_->pdf(wo, wh) / (4.0 * vect::dot(wo, wh));
    } else {
        // Transmission
        double eta = vect::cosTheta(wo) > 0.0 ? (etaB_ / etaA_) : (etaA_ / etaB_);
        Vector3d wh = vect::normalize(wo + wi * eta);
    
        double sqrtDenom = vect::dot(wo, wh) + eta * vect::dot(wi, wh);
        double dwhdwi = std::abs((eta * eta * vect::dot(wi, wh)) /
                                 (sqrtDenom * sqrtDenom));
        return distrib_->pdf(wo, wh) * dwhdwi;
    }
}

}  // namespace spica
