#define SPICA_API_EXPORT
#include "box.h"

namespace spica {

BoxFilter::BoxFilter()
    : Filter{} {
}

BoxFilter::BoxFilter(const RenderParams &params)
    : BoxFilter{Vector2d(params.getDouble("radius", 1.0), params.getDouble("radius", 1.0))} {
}

BoxFilter::BoxFilter(const Vector2d& radius)
    : Filter{ radius } {
}

BoxFilter::~BoxFilter() {
}

double BoxFilter::evaluate(const Point2d& p) const {
    return 1.0;
}

}  // namespace spica
