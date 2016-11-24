#define SPICA_API_EXPORT
#include "tent.h"

#include <algorithm>

#include "../core/point2d.h"
#include "../math/vector2d.h"

namespace spica {

TentFilter::TentFilter()
    : Filter{} {
}

TentFilter::TentFilter(const Vector2d& radius)
    : Filter{ radius } {
}

TentFilter::~TentFilter() {
}

double TentFilter::evaluate(const Point2d& p) const {
    return std::max(0.0, radius_.x() - std::abs(p.x())) *
           std::max(0.0, radius_.y() - std::abs(p.y()));
}

}  // namespace spica
