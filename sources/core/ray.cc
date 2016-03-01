#define SPICA_API_EXPORT
#include "ray.h"
#include "../core/common.h"

#include <cmath>

namespace spica {

Ray::Ray() {
}

Ray::Ray(const Point3d& origin, const Vector3d& direction, double maxDist,
         const Medium* medium)
    : org_{ origin }
    , dir_{ direction.normalized() }
    , invdir_{}
    , maxDist_{ maxDist }
    , medium_{ medium } {
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
    this->medium_  = ray.medium_;
    return *this;
}

void Ray::calcInvdir() {
    invdir_.xRef() = (dir_.x() == 0.0) ? INFTY : 1.0 / dir_.x();
    invdir_.yRef() = (dir_.y() == 0.0) ? INFTY : 1.0 / dir_.y();
    invdir_.zRef() = (dir_.z() == 0.0) ? INFTY : 1.0 / dir_.z();
}

}  // namespace spica
