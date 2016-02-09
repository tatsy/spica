#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECT_MATH_H_
#define _SPICA_VECT_MATH_H_

#include "../core/forward_decl.h"

namespace spica {

namespace vect {

    template <class T>
    double dot(const spica::Vector3_<T>& v, const spica::Normal3_<T>& n) {
        return v.x() * n.x() + v.y() * n.y() + v.z() * n.z();
    }

    template <class T>
    double dot(const spica::Normal3_<T>& n, const spica::Vector3_<T>& v) {
        return dot(v, n);
    }

    template <class T>
    double dot(const spica::Normal3_<T>& n1, const spica::Normal3_<T>& n2) {
        return n1.x() * n2.x() + n1.y() * n2.y() + n1.z() * n2.z();
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

// Point-Vector binary operation
template <class T>
spica::Point3_<T> operator+(const spica::Point3_<T>& p, const spica::Vector3_<T>& v) {
    return { p.x() + v.x(), p.y() + v.y(), p.z() + v.z() };
}

template <class T>
spica::Point3_<T> operator+(const spica::Point3_<T>& p1, const spica::Point3_<T>& p2) {
    return { p1.x() + p2.x(), p1.y() + p2.y(), p1.z() + p2.z() };
}

template <class T>
spica::Point3_<T> operator-(const spica::Point3_<T>& p, const spica::Vector3_<T>& v) {
    return { p.x() - v.x(), p.y() - v.y(), p.z() - v.z() };
}

template <class T>
spica::Vector3_<T> operator-(const spica::Point3_<T>& p1, const spica::Point3_<T>& p2) {
    return { p1.x() - p2.x(), p1.y() - p2.y(), p1.z() - p2.z() };
}

template <class T>
spica::Point3_<T> operator/(spica::Point3_<T>& p, T s) {
    Assertion(s != 0.0, "Zero division!!");
    return p * (1.0 / s);
}

#endif  // _SPICA_VECT_MATH_H_
