#define SPICA_API_EXPORT
#include "box.h"

namespace spica {

BoxFilter::BoxFilter()
    : Filter{} {
}

BoxFilter::BoxFilter(const Vector2D& radius)
    : Filter{ radius } {
}

BoxFilter::~BoxFilter() {
}

double BoxFilter::evaluate(const Point2D& p) const {
    return 1.0;
}

}  // namespace spica