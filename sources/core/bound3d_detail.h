#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BOUND3D_DETAIL_H_
#define _SPICA_BOUND3D_DETAIL_H_

#include "../renderer/ray.h"

namespace spica {

template <class T>
Bound3_<T>::Bound3_()
    : posMin_{}
    , posMax_{} {
    T minNum = std::numeric_limits<T>::lowest();
    T maxNum = std::numeric_limits<T>::max();
    posMin_ = Point3_<T>{ maxNum, maxNum, maxNum };
    posMax_ = Point3_<T>{ minNum, minNum, minNum };
}

template <class T>
Bound3_<T>::Bound3_(const Point3_<T>& posMin, const Point3_<T>& posMax)
    : posMin_{ posMin }
    , posMax_{ posMax } {
}

template <class T>
Bound3_<T>::Bound3_(const Bound3_<T>& b)
    : posMin_{ b.posMin_ }
    , posMax_{ b.posMax_ } {
}

template <class T>
Bound3_<T>::~Bound3_() {
}

template <class T>
Bound3_<T>& Bound3_<T>::operator=(const Bound3_<T>& b) {
    this->posMin_ = b.posMin_;
    this->posMax_ = b.posMax_;
    return *this;
}

template <class T>
bool Bound3_<T>::operator==(const Bound3_<T>& b) const {
    return posMin_ == b.posMin_ && posMax_ == b.posMax_;
}

template <class T>
bool Bound3_<T>::operator!=(const Bound3_<T>& b) const {
    return posMin_ != b.posMin_ || posMax_ != b.posMax_;
}

template <class T>
int Bound3_<T>::maximumExtent() const {
    Vector3_<T> diff = posMax_ - posMin_;
    if (diff.x() >= diff.y() && diff.x() >= diff.z()) return 0;
    if (diff.y() >= diff.z()) return 1;
    return 2;
}

template <class T>
bool Bound3_<T>::intersect(const Ray& ray, double* tNear, double* tFar) const {
    double t0 = 0.0, t1 = ray.maxDist();
    for (int i = 0; i < 3; i++) {
        
    }
}

template <class T>
Bound3_<T> Bound3_<T>::merge(const Bound3_<T>& b1, const Bound3_<T>& b2) {
    Bound3_<T> posMin = Point3_<T>::minimum(b1.posMin(), b2.posMin());
    Bound3_<T> posMax = Point3_<T>::maximum(b1.posMax(), b2.posMax());
    return { posMin, posMax };
}

template <class T>
Bound3_<T>& Bound3_<T>::merge(const Point3_<T>& p) {
    posMin_ = Point3_<T>::minimum(p, posMin_);
    posMax_ = Point3_<T>::maximum(p, posMax_);
    return *this;
}

template <class T>
T Bound3_<T>::area() const {
    Vector3_<T> diff = posMax_ - posMin_;
    const double xy = std::abs(diff.x() * diff.y());
    const double yz = std::abs(diff.y() * diff.z());
    const double zx = std::abs(diff.z() * diff.x());
    return 2.0 * (xy + yz + zx);
}

}  // namespace spica

#endif  // _SPICA_BOUND3D_DETAIL_H_
