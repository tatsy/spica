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
    virtual double D(const Vector3d& wh) const = 0;
    virtual double lambda(const Vector3d& w) const = 0;
    double G1(const Vector3d& w) const;
    double G(const Vector3d& wo, const Vector3d& wi) const;
    virtual Vector3d sample(const Vector3d &wo, const Point2d& rands) const = 0;
    double pdf(const Vector3d& wo, const Vector3d& wh) const;

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

    double D(const Vector3d& wh) const override;
    Vector3d sample(const Vector3d& wo, const Point2d& rands) const override;

    static double roughnessToAlpha(double rough);

private:
    // Private methods
    double lambda(const Vector3d& w) const override;
    static Vector3d sampleVA(const Vector3d& wi, double alphax, double alphay,
                             const Point2d& rands);
    static void sampleSlopes(double cosTheta, const Point2d& rands,
                             double* slopex, double* slopey);


    // Private fields
    const double alphax_, alphay_;

};  // class TrobridgeReitzDistribution
 
}  // namespace spica

#endif  // _SPICA_MICROFACET_H_
