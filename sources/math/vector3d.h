#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_VECTOR3D_H_
#define SPICA_VECTOR3D_H_

#include <iostream>
#include <array>
#include <type_traits>
#include <xmmintrin.h>
#include <immintrin.h>

#include "../core/common.h"

namespace spica {

    /** Three-dimensional vector.
     *  @ingroup math_module
     */
    template <class T>
    class Vector3_ {
    public:
        /** The Vector3D constructor.
         */
        Vector3_();

        /** The Vector3D constructor.
         * @param x x-coordinate
         * @param y y-coordinate
         * @param z z-coordinate
         */
        Vector3_(T x, T y, T z);

        /** The Vector3D copy constructor
         */
        Vector3_(const Vector3_& v);

        /** The Vector3D destructor.
         */
        virtual ~Vector3_();

        Vector3_& operator=(const Vector3_& v);
        Vector3_& operator+=(const Vector3_& v);
        Vector3_& operator+=(double x);
        Vector3_& operator-=(const Vector3_& v);
        Vector3_& operator-=(double x);
        Vector3_  operator-() const;

        /** Element-wise multiplication.
         */
        Vector3_& operator*=(const Vector3_& v);
        Vector3_& operator*=(double s);

        /** Element-wise division
         *  @param v multiplied vector
         */
        Vector3_& operator/=(const Vector3_& v);

        /** Scalar multiplication
         *  @param s multiplied scalar
         */
        Vector3_& operator/=(double s);

        bool operator==(const Vector3_& v) const;
        bool operator!=(const Vector3_& v) const;

        T dot(const Vector3_& v) const;
        static T dot(const Vector3_& v1, const Vector3_& v2);

        Vector3_ cross(const Vector3_& v) const;
        static Vector3_ cross(const Vector3_& v1, const Vector3_& v2);

        double norm() const;
        double squaredNorm() const;

        Vector3_ normalized() const;
        static Vector3_ normalize(const Vector3_& v);

        Vector3_ multiply(const Vector3_& v) const;

        static Vector3_ minimum(const Vector3_& v1, const Vector3_& v2);
        static Vector3_ maximum(const Vector3_& v1, const Vector3_& v2);

        T get(int d) const;

        std::string toString() const;

        /** Get x.
         */
        inline T x() const { return _x; }

        /** Get y.
         */
        inline T y() const { return _y; }

        /** Get z.
         */
        inline T z() const { return _z; }

        /** Get reference to x.
         */
        inline T& xRef() { return _x; }

        /** Get reference to y.
         */
        inline T& yRef() { return _y; }

        /** Get reference to z.
         */
        inline T& zRef() { return _z; }

    protected:
        T _x, _y, _z;

        static_assert(std::is_arithmetic<T>::value, "Template type must be arithmetic!!");

    };  // class Vector3D

    using Vector3D = Vector3_<double>;

}  // namespace spica

template <class T>
spica::Vector3_<T>
operator+(const spica::Vector3_<T>& v1, const spica::Vector3_<T>& v2);

template <class T>
spica::Vector3_<T>
operator-(const spica::Vector3_<T>& v1, const spica::Vector3_<T>& v2);

template <class T>
spica::Vector3_<T>
operator+(const spica::Vector3_<T>& v, T x);

template <class T>
spica::Vector3_<T>
operator+(T x, const spica::Vector3_<T>& v);

template <class T>
spica::Vector3_<T>
operator-(const spica::Vector3_<T>& v, T x);

template <class T>
spica::Vector3_<T>
operator-(T x, const spica::Vector3_<T>& v);

/** Element-wise multiplication
 */
template <class T>
spica::Vector3_<T>
operator*(const spica::Vector3_<T>& u, const spica::Vector3_<T>& v);

template <class T>
spica::Vector3_<T>
operator*(const spica::Vector3_<T>& v, double s);

template <class T>
spica::Vector3_<T>
operator*(double s, const spica::Vector3_<T>& v);

/** Element-wise division
 */
template <class T>
spica::Vector3_<T>
operator/(const spica::Vector3_<T>& u, const spica::Vector3_<T>& v);

template <class T>
spica::Vector3_<T>
operator/(const spica::Vector3_<T>& v, double s);

template <class T>
std::ostream& operator<<(std::ostream& os, const spica::Vector3_<T>& v);

// Include implementation header
#include "vector3d_detail.h"

#endif  // SPICA_VECTOR3D_H_
