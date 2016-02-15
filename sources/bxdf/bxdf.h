#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BXDF_H_
#define _SPICA_BXDF_H_

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/spectrum.h"

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

/**
 * The base class of BxDFs.
 */
class SPICA_EXPORTS BxDF {
public:
    // Public methods
    BxDF(BxDFType type = BxDFType::None);
    virtual ~BxDF();

    BxDF(const BxDF&) = default;
    BxDF& operator=(const BxDF&) = default;

    virtual Spectrum f(const Vector3D& wo, const Vector3D& wi) const = 0;
    virtual Spectrum sample(const Vector3D& wo, Vector3D* wi,
                            const Point2D& rands, double* pdf) const;
    virtual double pdf(const Vector3D& wo, const Vector3D& wi) const;

    inline BxDFType type() const { return type_; }

private:
    const BxDFType type_;

};  // class BxDF

/**
 * Lambertian refrection.
 */
class LambertianReflection : public BxDF {
public:
    LambertianReflection();
    LambertianReflection(const Spectrum& ref);

    LambertianReflection(const LambertianReflection&) = default;
    LambertianReflection& operator=(LambertianReflection&) = default;

    Spectrum f(const Vector3D& wo, const Vector3D& wi) const override;

private:
    Spectrum ref_;
};

/**
 * Specular reflection (Metal-like effect).
 */
class SpecularReflection : public BxDF {
};

/**
 * Fresnel specular refraction (Glass-like effect).
 */
class FresnelSpecular : public BxDF {

};

}  // namespace spica

#endif  // _SPICA_BXDF_H_