#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MICROFACET_H_
#define _SPICA_MICROFACET_H_

#include "core/common.h"
#include "core/core.hpp"

namespace spica {

/**
 * The base class for microfacet distributions.
 */
class SPICA_EXPORTS MicrofacetDistribution {
public:
    MicrofacetDistribution(double alphax, double alphay, bool sampleVisibleArea);
    virtual ~MicrofacetDistribution();
    virtual double D(const Vector3d& wh) const = 0;
    virtual double lambda(const Vector3d& w) const = 0;
    double G1(const Vector3d& w, const Vector3d &wh) const;
    double G(const Vector3d& wo, const Vector3d& wi, const Vector3d &wh) const;

    //! Sample half-vector wh
    virtual Vector3d sample(const Vector3d &wo, const Point2d& rands) const = 0;

    double pdf(const Vector3d& wo, const Vector3d& wh) const;
    inline double alphax() const { return alphax_; }
    inline double alphay() const { return alphay_; }

protected:
    const bool sampleVisibleArea_;
    const double alphax_, alphay_;
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
};

/**
 * Beckmann microfacet distribution
 */
class SPICA_EXPORTS BeckmannDistribution : public MicrofacetDistribution {
public:
    BeckmannDistribution(double alphax, double alphay, bool sampleVis = true);
    
    double D(const Vector3d &wh) const override;
    Vector3d sample(const Vector3d &wo, const Point2d &rands) const override;

    static double roughnessToAlpha(double rough);

private:
    // Private methods
    double lambda(const Vector3d &w) const override;
};
 
}  // namespace spica

#endif  // _SPICA_MICROFACET_H_
