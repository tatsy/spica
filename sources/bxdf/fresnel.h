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
    virtual Spectrum evaluate(double cosI) const = 0;
};

/** 
 * Fresnel conductor class.
 */
class FresnelConductor : public Fresnel {
};

/**
 * Fresnel dielectric class.
 */
class FresnelDielectric : public Fresnel {

};

}  // namespace spica

#endif  // _SPICA_FRESNEL_H_
