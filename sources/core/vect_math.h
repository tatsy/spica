#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECT_MATH_H_
#define _SPICA_VECT_MATH_H_

#include <type_traits>

#include "core/core.hpp"
#include "core/vector3d.h"

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
    const double nrm = std::sqrt(v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
    Assertion(nrm != 0.0, "Zero division!!");
    return { v.x() / nrm, v.y() / nrm, v.z() / nrm };
}

template <class Vec>
double sphericalPhi(const Vec& dir) {
    double p = std::atan2(dir.y(), dir.x());
    return p < 0.0 ? p + 2.0 * PI : p;
}

template <class Vec>
double sphericalTheta(const Vec& dir) {
    return std::acos(clamp(dir.z(), -1.0, 1.0));
}

template <class Vec>
double cosTheta(const Vec& w) {
    return w.z();
}

template <class Vec>
double cos2Theta(const Vec& w) {
    return w.z() * w.z();
}

template <class Vec>
double sin2Theta(const Vec& w) {
    return std::max(0.0, 1.0 - cos2Theta(w));
}

template <class Vec>
double sinTheta(const Vec& w) {
    return std::sqrt(sin2Theta(w));
}

template <class Vec>
double tanTheta(const Vec& w) {
    return sinTheta(w) / (cosTheta(w) + EPS);
}

template <class Vec>
double tan2Theta(const Vec& w) {
    return sin2Theta(w) / (cos2Theta(w) + EPS);
}

template <class Vec>
double cosPhi(const Vec& w) {
    double sinTheta = vect::sinTheta(w);
    return (sinTheta == 0.0) ? 1.0 : clamp(w.x() / sinTheta, -1.0, 1.0);
}

template <class Vec>
double sinPhi(const Vec& w) {
    double sinTheta = vect::sinTheta(w);
    return (sinTheta == 0.0) ? 0.0 : clamp(w.y() / sinTheta, -1.0, 1.0);    
}

template <class Vec>
double cos2Phi(const Vec& w) {
    const double c = vect::cosPhi(w);
    return c * c;
}

template <class Vec>
double sin2Phi(const Vec& w) {
    const double s = vect::sinPhi(w);
    return s * s;
}

template <class Vec>
bool sameHemisphere(const Vec& w1, const Vec& w2) {
    return w1.z() * w2.z() > 0.0;
}

template <class Vec>
void coordinateSystem(const Vec& w, Vec* u, Vec* v) {
    if (std::abs(w.x()) > std::abs(w.y())) {
        *u = vect::normalize(Vec(-w.z(), 0, w.x()));
    } else {
        *u = vect::normalize(Vec(0, w.z(), -w.y()));
    }
    *v = vect::normalize(vect::cross(w, *u));
}

/** 
 * Compute reflection vector of v w.r.t n
 * @param v incident direction
 * @param n normal of hitpoint
 * @return reflected direction
 */
template <class Vec1, class Vec2>
Vector3_<typename Vec1::type> reflect(const Vec1& wo, const Vec2& n) {
    using Type = typename Vec1::type;
    return Vector3_<Type>(- wo + Vector3_<Type>(n) * Type(2) * vect::dot(n, wo));
}

/** 
 * Compute refraction vector of v w.r.t n
 * @param v incident direction
 * @param n normal of hitpoint
 * @return true if refracted, false if totally reflected
 */
template <class Vec1, class Vec2>
bool refract(const Vec1& wi, const Vec2& n, double eta, Vec1* wt) {
    double cosThetaI = vect::dot(n, wi);
    double sin2ThetaI = std::max(0.0, 1.0 - cosThetaI * cosThetaI);
    double sin2ThetaT = eta * eta * sin2ThetaI;

    if (sin2ThetaT >= 1.0) return false;
    double cosThetaT = std::sqrt(1.0 - sin2ThetaT);
    *wt = eta * (-wi) + (eta * cosThetaI - cosThetaT) * Vec1(n);
    return true;
}

template <class Vec1, class Vec2>
inline Vec1 faceforward(const Vec1& n, const Vec2& v) {
    return vect::dot(n, v) < 0.0 ? -n : n;
}

}  // namespace vect

}  // namespace spica

#endif  // _SPICA_VECT_MATH_H_
