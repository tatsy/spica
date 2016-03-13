#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BOUND3D_DETAIL_H_
#define _SPICA_BOUND3D_DETAIL_H_

#include "ray.h"

namespace spica {

template <class T>
Bounds3_<T>::Bounds3_()
    : posMin_{}
    , posMax_{} {
    T minNum = std::numeric_limits<T>::lowest();
    T maxNum = std::numeric_limits<T>::max();
    posMin_ = Point3_<T>{ maxNum, maxNum, maxNum };
    posMax_ = Point3_<T>{ minNum, minNum, minNum };
}

template <class T>
Bounds3_<T>::Bounds3_(const Point3_<T>& posMin, const Point3_<T>& posMax)
    : posMin_{ posMin }
    , posMax_{ posMax } {
}

template <class T>
Bounds3_<T>::Bounds3_(const Bounds3_<T>& b)
    : posMin_{ b.posMin_ }
    , posMax_{ b.posMax_ } {
}

template <class T>
Bounds3_<T>::~Bounds3_() {
}

template <class T>
Bounds3_<T>& Bounds3_<T>::operator=(const Bounds3_<T>& b) {
    this->posMin_ = b.posMin_;
    this->posMax_ = b.posMax_;
    return *this;
}

template <class T>
bool Bounds3_<T>::operator==(const Bounds3_<T>& b) const {
    return posMin_ == b.posMin_ && posMax_ == b.posMax_;
}

template <class T>
bool Bounds3_<T>::operator!=(const Bounds3_<T>& b) const {
    return posMin_ != b.posMin_ || posMax_ != b.posMax_;
}

template <class T>
int Bounds3_<T>::maximumExtent() const {
    Vector3_<T> diff = posMax_ - posMin_;
    if (diff.x() >= diff.y() && diff.x() >= diff.z()) return 0;
    if (diff.y() >= diff.z()) return 1;
    return 2;
}

template <class T>
bool Bounds3_<T>::intersect(const Ray& ray, double* tNear, double* tFar) const {
    double t0 = 0.0, t1 = ray.maxDist();
    for (int i = 0; i < 3; i++) {
        double tt0 = (posMin_[i] - ray.org()[i]) * ray.invdir()[i];
        double tt1 = (posMax_[i] - ray.org()[i]) * ray.invdir()[i];
        if (tt0 > tt1) std::swap(tt0, tt1);

        t0 = std::max(t0, tt0);
        t1 = std::min(t1, tt1);
        if (t0 > t1) return false;
    }

    if (tNear) *tNear = t0;
    if (tFar)  *tFar  = t1;
    return true;
}

template <class T>
Bounds3_<T> Bounds3_<T>::merge(const Bounds3_<T>& b1, const Bounds3_<T>& b2) {
    Point3_<T> posMin = Point3_<T>::minimum(b1.posMin(), b2.posMin());
    Point3_<T> posMax = Point3_<T>::maximum(b1.posMax(), b2.posMax());
    return Bounds3_<T>{ posMin, posMax };
}

template <class T>
void Bounds3_<T>::merge(const Bounds3_<T>& b) {
    posMin_ = Point3_<T>::minimum(posMin_, b.posMin_);
    posMax_ = Point3_<T>::maximum(posMax_, b.posMax_);
}

template <class T>
void Bounds3_<T>::merge(const Point3_<T>& p) {
    posMin_ = Point3_<T>::minimum(p, posMin_);
    posMax_ = Point3_<T>::maximum(p, posMax_);
}

template <class T>
bool Bounds3_<T>::inside(const Point3_<T>& p) const {
    return (posMin_.x() < p.x() && p.x() < posMax_.x() &&
            posMin_.x() < p.y() && p.x() < posMax_.y() &&
            posMin_.x() < p.z() && p.x() < posMax_.z());
}

template <class T>
T Bounds3_<T>::area() const {
    Vector3_<T> diff = posMax_ - posMin_;
    const double xy = std::abs(diff.x() * diff.y());
    const double yz = std::abs(diff.y() * diff.z());
    const double zx = std::abs(diff.z() * diff.x());
    return 2.0 * (xy + yz + zx);
}

}  // namespace spica

#endif  // _SPICA_BOUND3D_DETAIL_H_
