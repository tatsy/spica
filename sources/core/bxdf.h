#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BXDF_H_
#define _SPICA_BXDF_H_

#include <memory>

#include "core/core.hpp"
#include "core/common.h"
#include "core/uncopyable.h"
#include "core/spectrum.h"

#include "core/render.hpp"
#include "core/fresnel.h"

namespace spica {

enum class BxDFType : int {
    None         = 0x00,
    Reflection   = 0x01,
    Transmission = 0x02,
    Diffuse      = 0x04,
    Glossy       = 0x08,
    Specular     = 0x10,
    All          = Reflection | Transmission | Diffuse | Glossy | Specular,
};

inline BxDFType operator|(BxDFType t1, BxDFType t2) {
    return static_cast<BxDFType>(static_cast<int>(t1) | static_cast<int>(t2));
}

inline BxDFType operator&(BxDFType t1, BxDFType t2) {
    return static_cast<BxDFType>(static_cast<int>(t1) & static_cast<int>(t2));
}

inline BxDFType operator~(BxDFType t) {
    return static_cast<BxDFType>(~static_cast<int>(t));
}

/**
 * The base class of BxDFs.
 */
class SPICA_EXPORTS BxDF : public Uncopyable {
public:
    // Public methods
    explicit BxDF(BxDFType type = BxDFType::None);
    virtual ~BxDF();

    virtual Spectrum f(const Vector3d& wo, const Vector3d& wi) const = 0;
    virtual Spectrum sample(const Vector3d& wo, Vector3d* wi,
                            const Point2d& rands, double* pdf,
                            BxDFType* sampledType = nullptr) const;
    virtual double pdf(const Vector3d& wo, const Vector3d& wi) const;

    inline BxDFType type() const { return type_; }

private:
    const BxDFType type_;

};  // class BxDF

/**
 * Lambertian refrection.
 */
class SPICA_EXPORTS LambertianReflection : public BxDF {
public:
    explicit LambertianReflection(const Spectrum& ref);

    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;

private:
    Spectrum ref_;
};

/**
 * Lambertian transmission.
 */
class SPICA_EXPORTS LambertianTransmission : public BxDF {
public:
    explicit LambertianTransmission(const Spectrum& tr);

    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;

private:
    Spectrum tr_;
};

/**
 * Specular reflection (Metal-like effect).
 */
class SPICA_EXPORTS SpecularReflection : public BxDF {
public:
    // Public methods
    SpecularReflection(const Spectrum& ref, Fresnel* fresnel);

    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;
    Spectrum sample(const Vector3d& wo, Vector3d* wi, const Point2d& rands,
                    double* pdf, BxDFType* sampledType) const override;
    double pdf(const Vector3d& wo, const Vector3d& wi) const override;

private:
    // Private fields
    Spectrum ref_;
    Fresnel* fresnel_;
};

/**
 * Specular transmission.
 */
class SPICA_EXPORTS SpecularTransmission : public BxDF {
public:
    // Public methods
    SpecularTransmission(const Spectrum& tr, double etaA, double etaB);

    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;
    Spectrum sample(const Vector3d& wo, Vector3d* wi, const Point2d& rands,
                    double* pdf, BxDFType* sampledType) const override;
    double pdf(const Vector3d& wo, const Vector3d& wi) const override;

private:
    // Private fields
    Spectrum tr_;
    double etaA_, etaB_;
    std::unique_ptr<FresnelDielectric> fresnel_;

};  // class SpecularTransmission

/**
 * Fresnel specular refraction (Glass-like effect).
 */
class SPICA_EXPORTS FresnelSpecular : public BxDF {
public:
    // Public methods
    FresnelSpecular();
    FresnelSpecular(const Spectrum& ref, const Spectrum& tr, double etaA, double etaB);

    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;
    Spectrum sample(const Vector3d& wo, Vector3d* wi, const Point2d& rands,
                    double* pdf, BxDFType* sampledType) const override;
    double pdf(const Vector3d& wo, const Vector3d& wi) const override;

private:
    // Private fields
    Spectrum ref_;
    Spectrum tr_;
    double etaA_ = 1.0;
    double etaB_ = 1.0;
};

/**
 * Microfacet reflaction.
 */
class SPICA_EXPORTS MicrofacetReflection : public BxDF {
public:
    MicrofacetReflection(const Spectrum& ref,
                         MicrofacetDistribution* distrib, Fresnel* fresnel);
    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;
    Spectrum sample(const Vector3d& wo, Vector3d* wi, const Point2d& rands,
                    double* pdf, BxDFType* sampledType) const override;
    double pdf(const Vector3d& wo, const Vector3d& wi) const override;

private:
    const Spectrum ref_;
    const MicrofacetDistribution* distrib_;
    const Fresnel* fresnel_;
};

/**
 * Microfacet transmission.
 */
class SPICA_EXPORTS MicrofacetTransmission : public BxDF {
public:
    // Public methods
    MicrofacetTransmission(const Spectrum &re, const Spectrum& tr,
                           MicrofacetDistribution* distrib, double etaA,
                           double etaB);
    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;
    Spectrum f(const Vector3d &wo, const Vector3d &wi, const Vector3d &wh) const;
    Spectrum sample(const Vector3d& wo, Vector3d* wi, const Point2d& rands,
                    double* pdf, BxDFType* sampledType) const override;
    double pdf(const Vector3d& wo, const Vector3d& wi) const override;
    double pdf(const Vector3d &wo, const Vector3d &wi, const Vector3d &wh) const;

private:
    // Private fields
    const Spectrum re_;
    const Spectrum tr_;
    const MicrofacetDistribution* distrib_;
    const double etaA_, etaB_;
};

}  // namespace spica

#endif  // _SPICA_BXDF_H_