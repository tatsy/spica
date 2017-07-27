#define SPICA_API_EXPORT
#include "gaussian.h"

#include <cmath>
#include <algorithm>

#include "../core/point2d.h"

namespace spica {

GaussianFilter::GaussianFilter()
    : Filter{}
    , beta_{ 0.0 }
    , expX_{ 0.0 }
    , expY_{ 0.0 } {
} 

GaussianFilter::GaussianFilter(const RenderParams &params)
    : GaussianFilter{Vector2d(params.getDouble("radius", 1.0), params.getDouble("radius", 1.0)),
                     params.getDouble("sigma", 0.5)} {
}
 
GaussianFilter::GaussianFilter(const Vector2d& radius, double sigma)
    : Filter{ radius }
    , beta_{ 1.0 / sigma }
    , expX_{ std::exp(-radius.x() * radius.x() * beta_) }
    , expY_{ std::exp(-radius.y() * radius.y() * beta_) } {
}

GaussianFilter::~GaussianFilter() {
}

double GaussianFilter::evaluate(const Point2d& p) const {
    return gaussFunc(p.x(), expX_) * gaussFunc(p.y(), expY_);
}

double GaussianFilter::gaussFunc(double d, double expv) const {
    return std::max(0.0, std::exp(-beta_ * d * d) - expv);
}

}  // namespace spica
