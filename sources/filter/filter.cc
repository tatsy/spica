#define SPICA_API_EXPORT
#include "filter.h"

namespace spica {

Filter::Filter()
    : radius_{}
    , invRad_{} {
}

Filter::Filter(const Vector2D& radius)
    : radius_{ radius }
    , invRad_{ 1.0 / radius.x(), 1.0 / radius.y() } {
}

Filter::~Filter() {
}

}  // namespace spica
