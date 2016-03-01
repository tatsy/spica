#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BOUND3D_H_
#define _SPICA_BOUND3D_H_

#include <type_traits>

#include "point3d.h"
#include "ray.h"

namespace spica {

template <class T>
class Bounds3_ {
public:
    // Public methods
    Bounds3_();
    Bounds3_(const Point3_<T>& posMin, const Point3_<T>& posMax);
    Bounds3_(const Bounds3_<T>& b);

    virtual ~Bounds3_();

    Bounds3_<T>& operator=(const Bounds3_<T>& b);
    bool operator==(const Bounds3_<T>& b) const;
    bool operator!=(const Bounds3_<T>& b) const;

    /** Maximum extent: return 0 -> x, 1 -> y, 2 -> z. */
    int maximumExtent() const;

    /** Intersection test. */
    bool intersect(const Ray& ray, double* tNear = nullptr,
                   double* tFar = nullptr) const;

    //** Merge two bounds. */
    static Bounds3_<T> merge(const Bounds3_<T>& b1, const Bounds3_<T>& b2);

    void merge(const Bounds3_<T>& b);
    void merge(const Point3_<T>& p);

    T area() const;

    inline Point3_<T> posMin() const { return posMin_; }
    inline Point3_<T> posMax() const { return posMax_; }

private:
    // Private fields
    Point3_<T> posMin_;
    Point3_<T> posMax_;

    static_assert(std::is_arithmetic<T>::value,
                  "Template type must be arithmetic!!");
};

using Bounds3i = Bounds3_<int>;
using Bounds3f = Bounds3_<float>;
using Bounds3d = Bounds3_<double>;

}  // namespace spica

#include "bounds3d_detail.h"

#endif  // _SPICA_BOUND3D_H_
