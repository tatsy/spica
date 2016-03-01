#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PHASE_H_
#define _SPICA_PHASE_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

namespace spica {

namespace phase {

SPICA_EXPORTS double hg(double cosTheta, double g);

}  // namespace phase

class PhaseFunction {
public:
    virtual double p(const Vector3d& wo, const Vector3d& wi) const = 0;
    virtual double sample(const Vector3d& wo, Vector3d* wi,
                          const Point2d& rands) const = 0;
};

class HenyeyGreenstein : public PhaseFunction {
public:
    HenyeyGreenstein(double g);
    double p(const Vector3d& wo, const Vector3d& wi) const;
    double sample(const Vector3d& wo, Vector3d* wi,
                  const Point2d& rands) const;

private:
    const double g_;
};

}  // namespace spica

#endif  // _SPICA_PHASE_H_
