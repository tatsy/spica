#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FRESNEL_H_
#define _SPICA_FRESNEL_H_

#include "../core/spectrum.h"

namespace spica {

/**
 * The inteface for Fresnel reflections.
 */
class Fresnel {
public:
    virtual ~Fresnel() {}
    virtual Spectrum evaluate(double cosThetaI) const = 0;
};

/** 
 * Fresnel conductor class.
 */
class FresnelConductor : public Fresnel {
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
class FresnelDielectric : public Fresnel {
public:
    FresnelDielectric(double etaI, double etaT);
    Spectrum evaluate(double cosThetaI) const override;

private:
    double etaI_, etaT_;
};

}  // namespace spica

#endif  // _SPICA_FRESNEL_H_
