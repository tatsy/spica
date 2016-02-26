#define SPICA_API_EXPORT
#include "phase.h"

#include <cmath>

namespace spica {

namespace phase {

double hg(double cosTheta, double g) {
    const double g2 = g * g;
    const double denom = 1.0 + g2 - 2.0 * g * cosTheta;
    return (1.0 / (4.0 * PI)) * (1.0 - g2) / (denom * std::sqrt(denom));
}

}  // namespace phase

}  // namespace spica