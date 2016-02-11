#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECT_MATH_H_
#define _SPICA_VECT_MATH_H_

#include "../core/forward_decl.h"

namespace spica {

namespace vect {

    template <class Vec1, class Vec2>
    double dot(const Vec1& v1, const Vec2& v2) {
        return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
    }

    template <class Vec1, class Vec2>
    double absDot(const Vec1& v1, const Vec2& v2) {
        return std::abs(dot(v1, v2));
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
