#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FRESNEL_H_
#define _SPICA_FRESNEL_H_

#include "../core/common.h"
#include "../core/spectrum.h"

namespace spica {

/**
 * The inteface for Fresnel reflections.
 */
class SPICA_EXPORTS Fresnel {
public:
    virtual ~Fresnel() {}
    virtual Spectrum evaluate(double cosThetaI) const = 0;
};

/** 
 * Fresnel conductor class.
 */
class SPICA_EXPORTS FresnelConductor : public Fresnel {
public:
    FresnelConductor(const Spectrum& etaI, const Spectrum& etaT,
                     const Spectrum& k);
    Spectrum evaluate(double cosThetaI) const override;

private:
    Spectrum etaI_, etaT_, k_;
};

/**
 * Fresnel dielectric class.
 */
class SPICA_EXPORTS FresnelDielectric : public Fresnel {
public:
    FresnelDielectric(double etaI, double etaT);
    Spectrum evaluate(double cosThetaI) const override;

private:
    double etaI_, etaT_;
};

/**
 * Dammy Fresnel class which returns always one.
 */
class SPICA_EXPORTS FresnelNoOp : public Fresnel {
public:
    FresnelNoOp() {}
    Spectrum evaluate(double cosThetaI) const override {
        return Spectrum(1.0);
    }
};

// -----------------------------------------------------------------------------
// Fresnel utility functions
// -----------------------------------------------------------------------------

SPICA_EXPORTS Spectrum FrConductor(double cosThetaI, const Spectrum& etaI,
                                   const Spectrum& etaT, const Spectrum& k);
SPICA_EXPORTS double FrDielectric(double cosThetaI, double etaI, double etaT);

SPICA_EXPORTS double FrDiffuseReflectance(double eta);

}  // namespace spica

#endif  // _SPICA_FRESNEL_H_
