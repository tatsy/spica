#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_GAUSSIAN_H_
#define _SPICA_GAUSSIAN_H_

#include "core/common.h"
#include "core/filter.h"
#include "core/renderparams.h"

namespace spica {

class SPICA_EXPORTS GaussianFilter : public Filter {
public:
    // Public methods
    GaussianFilter();
    explicit GaussianFilter(RenderParams &params);
    GaussianFilter(const Vector2d& radius, double sigma);
    GaussianFilter(const GaussianFilter&) = default;
    ~GaussianFilter();

    GaussianFilter& operator=(const GaussianFilter&) = default;

    double evaluate(const Point2d& p) const override;

private:
    // Private methods
    double gaussFunc(double d, double expv) const;

    // Private fields
    double beta_;
    double expX_, expY_;
};

SPICA_EXPORT_PLUGIN(GaussianFilter, "Guassian reconstruction filter");

}  // namespace spica

#endif  // _SPICA_GAUSSIAN_H_
