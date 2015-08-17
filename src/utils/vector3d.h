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

    class SPICA_VECTOR3D_DLL Vector3D : public AxisComparable {
    protected:

#ifndef __AVX__
        std::array<double, 3> _xyz;
#else
        union m256d {
            __m256d m;
            align_attrib(double, 32) v[4];
        };
        m256d _xyz;
#endif

    public:
        Vector3D();
        Vector3D(double x, double y, double z);
        Vector3D(const Vector3D& v);
        ~Vector3D();

        Vector3D& operator=(const Vector3D& v);
        Vector3D& operator+=(const Vector3D& v);
        Vector3D& operator+=(double x);
        Vector3D& operator-=(const Vector3D& v);
        Vector3D& operator-=(double x);
        Vector3D  operator-() const;

        // Element-wise multiplication
        Vector3D& operator*=(const Vector3D& v);
        Vector3D& operator*=(double s);

        // Element-wise division
        Vector3D& operator/=(const Vector3D& v);
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

        // Compute reflection vector of v w.r.t n
        // @param[in] v: a vector to be reflected
        // @param[in] n: a normal vector of reflection point
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

// Element-wise multiplication
SPICA_VECTOR3D_DLL spica::Vector3D operator*(const spica::Vector3D& u, const spica::Vector3D& v);

SPICA_VECTOR3D_DLL spica::Vector3D operator*(const spica::Vector3D& v, double s);
SPICA_VECTOR3D_DLL spica::Vector3D operator*(double s, const spica::Vector3D& v);

// Element-wise division
SPICA_VECTOR3D_DLL spica::Vector3D operator/(const spica::Vector3D& u, const spica::Vector3D& v);

SPICA_VECTOR3D_DLL spica::Vector3D operator/(const spica::Vector3D& v, double s);

SPICA_VECTOR3D_DLL std::ostream& operator<<(std::ostream& os, const spica::Vector3D v);

#endif  // SPICA_VECTOR3D_H_
