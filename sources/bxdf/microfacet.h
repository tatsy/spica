#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MICROFACET_H_
#define _SPICA_MICROFACET_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

namespace spica {

/**
 * The base class for microfacet distributions.
 */
class SPICA_EXPORTS MicrofacetDistribution {
public:
    virtual ~MicrofacetDistribution();
    virtual double D(const Vector3D& wh) const = 0;
    virtual double lambda(const Vector3D& w) const = 0;
    double G1(const Vector3D& w) const;
    double G(const Vector3D& wo, const Vector3D& wi) const;
    virtual Vector3D sample(const Vector3D &wo, const Point2D& rands) const = 0;
    double pdf(const Vector3D& wo, const Vector3D& wh) const;

protected:
    explicit MicrofacetDistribution(bool sampleVisibleArea);
    const bool sampleVisibleArea_;
};

/**
 * Trowbridge and Reitz's microfacet distribution (GGX)
 */
class SPICA_EXPORTS TrowbridgeReitzDistribution : public MicrofacetDistribution {
public:
    TrowbridgeReitzDistribution(double alphax, double alphay,
                                bool samplevis = true);

    double D(const Vector3D& wh) const override;
    Vector3D sample(const Vector3D& wo, const Point2D& rands) const override;

    static double roughnessToAlpha(double rough);

private:
    // Private methods
    double lambda(const Vector3D& w) const;
    static Vector3D sampleVA(const Vector3D& wi, double alphax, double alphay,
                             const Point2D& rands);
    static void sampleSlopes(double cosTheta, const Point2D& rands,
                             double* slopex, double* slopey);


    // Private fields
    const double alphax_, alphay_;

};  // class TrobridgeReitzDistribution
 
}  // namespace spica

#endif  // _SPICA_MICROFACET_H_
