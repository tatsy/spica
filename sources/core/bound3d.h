#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BOUND3D_H_
#define _SPICA_BOUND3D_H_

#include <type_traits>

#include "../core/point3d.h"
#include "../renderer/ray.h"

namespace spica {

template <class T>
class Bound3_ {
public:
    // Public methods
    Bound3_();
    Bound3_(const Point3_<T>& posMin, const Point3_<T>& posMax);
    Bound3_(const Bound3_<T>& b);

    virtual ~Bound3_();

    Bound3_<T>& operator=(const Bound3_<T>& b);
    bool operator==(const Bound3_<T>& b) const;
    bool operator!=(const Bound3_<T>& b) const;

    //! Maximum extent: return 0 -> x, 1 -> y, 2 -> z.
    int maximumExtent() const;

    //! Intersection test.
    bool intersect(const Ray& ray, double* tNear = nullptr,
                   double* tFar = nullptr) const;

    //! Merge two bounds.
    static Bound3_<T> merge(const Bound3_<T>& b1, const Bound3_<T>& b2);

    void merge(const Bound3_<T>& b);
    void merge(const Point3_<T>& p);

    T area() const;

    inline Point3_<T> posMin() const { return posMin_; }
    inline Point3_<T> posMax() const { return posMax_; }

private:
    // Private fields
    Point3_<T> posMin_;
    Point3_<T> posMax_;

    static_assert(std::is_arithmetic<T>::value, "Template type must be arithmetic!!");
};

using Bound3i = Bound3_<int>;
using Bound3f = Bound3_<float>;
using Bound3d = Bound3_<double>;

}  // namespace spica

#include "bound3d_detail.h"

#endif  // _SPICA_BOUND3D_H_
