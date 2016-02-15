#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECT_MATH_H_
#define _SPICA_VECT_MATH_H_

#include <type_traits>

#include "../core/forward_decl.h"

namespace spica {

namespace vect {

template <class Vec1, class Vec2>
typename Vec1::type dot(const Vec1& v1, const Vec2& v2) {
    static_assert(std::is_same<typename Vec1::type, typename Vec2::type>::value,
                  "Base types of two vectors must be the same!!");
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}

template <class Vec1, class Vec2>
double absDot(const Vec1& v1, const Vec2& v2) {
    return std::abs(dot(v1, v2));
}

template <class Vec>
Vec cross(const Vec& v1, const Vec& v2) {
    using Type = typename Vec::type;
    Type x = v1.y() * v2.z() - v1.z() * v2.y();
    Type y = v1.z() * v2.x() - v1.x() * v2.z();
    Type z = v1.x() * v2.y() - v1.y() * v2.x();
    return { x, y, z };
}

template <class Vec>
Vec normalize(const Vec& v) {
    const double nrm = (v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
    Assertion(nrm != 0.0, "Zero division!!");
    return { v.x() / nrm, v.y() / nrm, v.z() / nrm };
}

template <class Vec>
double sphericalPhi(const Vec& dir) {
    double p = atan2(dir.y(), dir.x());
    return p < 0.0 ? p + 2.0 * PI : p;
}

template <class Vec>
double sphericalTheta(const Vec& dir) {
    return acos(clamp(dir.z(), -1.0, 1.0));
}

template <class Vec>
void coordinateSystem(const Vec& w, Vec* u, Vec* v) {
    if (std::abs(w.x()) > 0.1) {
        *u = vect::normalize(vect::cross(Vec(0.0, 1.0, 0.0), w));
    } else {
        *u = vect::normalize(vect::cross(Vec(1.0, 0.0, 0.0), w));
    }
    *v = vect::normalize(vect::cross(w, *u));
}

/** 
 * Compute reflection vector of v w.r.t n
 * @param v incident direction
 * @param n normal of hitpoint
 * @return reflected direction
 */
template <class T>
Vector3_<T> reflect(const Vector3_<T>& v, const Normal3_<T>& n) {
    return (v - Vector3D(n) * T(2) * vect::dot(n, v));        
}

}  // namespace vect

}  // namespace spica

#endif  // _SPICA_VECT_MATH_H_
