#define SPICA_API_EXPORT
#include "phase.h"

#include <cmath>

#include "core/point2d.h"
#include "core/vector3d.h"
#include "core/vect_math.h"

namespace spica {

namespace phase {

double hg(double cosTheta, double g) {
    const double g2 = g * g;
    const double denom = 1.0 + g2 - 2.0 * g * cosTheta;
    return (1.0 / (4.0 * PI)) * (1.0 - g2) / (denom * std::sqrt(denom));
}

}  // namespace phase

HenyeyGreenstein::HenyeyGreenstein(double g)
    : g_{ g } {
}

double HenyeyGreenstein::p(const Vector3d& wo, const Vector3d& wi) const {
    return phase::hg(vect::dot(wo, wi), g_);
}

double HenyeyGreenstein::sample(const Vector3d& wo, Vector3d* wi,
                                const Point2d& rands) const {
    double cosTheta;
    if (std::abs(g_) < EPS) {
        cosTheta = 1.0 - 2.0 * rands[0];
    } else {
        double tmp = (1.0 - g_ * g_) / (1.0 - g_ + 2.0 * g_ * rands[0]);
        cosTheta = (1.0 + g_ * g_ - tmp * tmp) / (2.0 * g_);
    }

    double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
    double phi = 2.0 * PI * rands[1];
    Vector3d u, v;
    vect::coordinateSystem(wo, &u, &v);

    *wi = cos(phi) * sinTheta * u + sin(phi) * sinTheta * v -
          cosTheta * wo;
    return phase::hg(-cosTheta, g_);
}

}  // namespace spica