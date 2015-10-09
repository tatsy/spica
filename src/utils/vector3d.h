#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_VECTOR3D_H_
#define SPICA_VECTOR3D_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_VECTOR3D_EXPORT
        #define SPICA_VECTOR3D_DLL __declspec(dllexport)
    #else
        #define SPICA_VECTOR3D_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_VECTOR3D_DLL
#endif

#include <iostream>
#include <array>
#include <xmmintrin.h>
#include <immintrin.h>

#include "common.h"
#include "axis_comparable.h"

namespace spica {

    /** Three-dimensional vector class.
     */
    class SPICA_VECTOR3D_DLL Vector3D : public AxisComparable {
    protected:

#ifndef __AVX__
        double _x, _y, _z;
#else
        union m256d {
            __m256d m;
            align_attrib(double, 32) v[4];
        };
        m256d _xyz;
#endif

    public:
        /** The Vector3D constructor.
         */
        Vector3D();

        /** The Vector3D constructor.
         * @param x x-coordinate
         * @param y y-coordinate
         * @param z z-coordinate
         */
        Vector3D(double x, double y, double z);

        /** The Vector3D constructor.
         * @param xyz static array including x, y, z coordinates
         */
        Vector3D(const std::array<double, 3>& xyz);

        /** The Vector3D copy constructor
         */
        Vector3D(const Vector3D& v);

        /** The Vector3D destructor.
         */
        virtual ~Vector3D();

        Vector3D& operator=(const Vector3D& v);
        Vector3D& operator+=(const Vector3D& v);
        Vector3D& operator+=(double x);
        Vector3D& operator-=(const Vector3D& v);
        Vector3D& operator-=(double x);
        Vector3D  operator-() const;

        /** Element-wise multiplication.
         */
        Vector3D& operator*=(const Vector3D& v);
        Vector3D& operator*=(double s);

        /** Element-wise division
         * @param v multiplied vector
         */
        Vector3D& operator/=(const Vector3D& v);

        /** Scalar multiplication
         * @param s multiplied scalar
         */
        Vector3D& operator/=(double s);

        bool operator==(const Vector3D& v) const;

        double dot(const Vector3D& v) const;
        static double dot(const Vector3D& v1, const Vector3D& v2);

        Vector3D cross(const Vector3D& v) const;
        static Vector3D cross(const Vector3D& v1, const Vector3D& v2);

        double norm() const;
        double squaredNorm() const;

        Vector3D normalized() const;
        static Vector3D normalize(const Vector3D& v);

        Vector3D multiply(const Vector3D& v) const;

        static Vector3D minimum(const Vector3D& v1, const Vector3D& v2);
        static Vector3D maximum(const Vector3D& v1, const Vector3D& v2);

        /** Compute reflection vector of v w.r.t n
         * @param v incident direction
         * @param n normal of hitpoint
         * @return reflected direction
         */
        static Vector3D reflect(const Vector3D& v, const Vector3D& n);

        double get(int d) const;

        std::string toString() const;

        double x() const;
        double y() const;
        double z() const;
        double& x();
        double& y();
        double& z();

    };  // class Vector3D

}  // namespace spica

SPICA_VECTOR3D_DLL spica::Vector3D operator+(const spica::Vector3D& v1, const spica::Vector3D& v2);
SPICA_VECTOR3D_DLL spica::Vector3D operator-(const spica::Vector3D& v1, const spica::Vector3D& v2);

SPICA_VECTOR3D_DLL spica::Vector3D operator+(const spica::Vector3D& v, double x);
SPICA_VECTOR3D_DLL spica::Vector3D operator+(double x, const spica::Vector3D& v);
SPICA_VECTOR3D_DLL spica::Vector3D operator-(const spica::Vector3D& v, double x);
SPICA_VECTOR3D_DLL spica::Vector3D operator-(double x, const spica::Vector3D& v);

/** Element-wise multiplication
 */
SPICA_VECTOR3D_DLL spica::Vector3D operator*(const spica::Vector3D& u, const spica::Vector3D& v);

SPICA_VECTOR3D_DLL spica::Vector3D operator*(const spica::Vector3D& v, double s);
SPICA_VECTOR3D_DLL spica::Vector3D operator*(double s, const spica::Vector3D& v);

/** Element-wise division
 */
SPICA_VECTOR3D_DLL spica::Vector3D operator/(const spica::Vector3D& u, const spica::Vector3D& v);

SPICA_VECTOR3D_DLL spica::Vector3D operator/(const spica::Vector3D& v, double s);

SPICA_VECTOR3D_DLL std::ostream& operator<<(std::ostream& os, const spica::Vector3D v);

#endif  // SPICA_VECTOR3D_H_
