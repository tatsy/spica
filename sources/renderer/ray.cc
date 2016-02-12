#define SPICA_API_EXPORT
#include "ray.h"
#include "../core/common.h"

#include <cmath>

namespace spica {

Ray::Ray() {
}

Ray::Ray(const Point& origin, const Vector3D& direction, double maxDist)
    : org_{ origin }
    , dir_{ direction }
    , invdir_{}
    , maxDist_{ maxDist } {
    Assertion(std::abs(1.0 - direction.norm()) < EPS,
                "Direction must be unit vector!!");
    calcInvdir();
}

Ray::Ray(const Ray& ray)
    : Ray {} {
    operator=(ray);
}

Ray::~Ray() {
}

Ray& Ray::operator=(const Ray& ray) {
    this->org_     = ray.org_;
    this->dir_     = ray.dir_;
    this->invdir_  = ray.invdir_;
    this->maxDist_ = ray.maxDist_;
    return *this;
}

void Ray::calcInvdir() {
    invdir_.xRef() = (dir_.x() == 0.0) ? INFTY : 1.0 / dir_.x();
    invdir_.yRef() = (dir_.y() == 0.0) ? INFTY : 1.0 / dir_.y();
    invdir_.zRef() = (dir_.z() == 0.0) ? INFTY : 1.0 / dir_.z();
}

}  // namespace spica
