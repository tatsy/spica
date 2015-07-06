#ifndef SPICA_VECTOR3_H_
#define SPICA_VECTOR3_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_VECTOR3_EXPORT
        #define SPICA_VECTOR3_DLL __declspec(dllexport)
    #else
        #define SPICA_VECTOR3_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_VECTOR3_DLL
#endif

#include <iostream>
#include <array>
#include <xmmintrin.h>
#include <immintrin.h>

#include "common.h"
#include "axis_comparable.h"

namespace spica {

    class SPICA_VECTOR3_DLL Vector3 : public AxisComparable {
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
        Vector3();
        Vector3(double x, double y, double z);
        Vector3(const Vector3& v);
        ~Vector3();

        Vector3& operator=(const Vector3& v);
        Vector3& operator+=(const Vector3& v);
        Vector3& operator-=(const Vector3& v);
        Vector3  operator-() const;
        Vector3& operator*=(double s);
        Vector3& operator/=(double s);
        bool operator==(const Vector3& v) const;

        double dot(const Vector3& v) const;
        static double dot(const Vector3& v1, const Vector3& v2);

        Vector3 cross(const Vector3& v) const;
        static Vector3 cross(const Vector3& v1, const Vector3& v2);

        double norm() const;
        double squaredNorm() const;

        Vector3 normalized() const;
        static Vector3 normalize(const Vector3& v);

        Vector3 multiply(const Vector3& v) const;

        static Vector3 minimum(const Vector3& v1, const Vector3& v2);
        static Vector3 maximum(const Vector3& v1, const Vector3& v2);

        // Compute reflection vector of v w.r.t n
        // @param[in] v: a vector to be reflected
        // @param[in] n: a normal vector of reflection point
        static Vector3 reflect(const Vector3& v, const Vector3& n);

        double get(int d) const;

        std::string toString() const;

        inline double x() const;
        inline double y() const;
        inline double z() const;
        inline double& x();
        inline double& y();
        inline double& z();

    };  // class Vector3

}  // namespace spica

SPICA_VECTOR3_DLL spica::Vector3 operator+(const spica::Vector3& v1, const spica::Vector3& v2);
SPICA_VECTOR3_DLL spica::Vector3 operator-(const spica::Vector3& v1, const spica::Vector3& v2);
SPICA_VECTOR3_DLL spica::Vector3 operator*(const spica::Vector3& v, double s);
SPICA_VECTOR3_DLL spica::Vector3 operator*(double s, const spica::Vector3& v);
SPICA_VECTOR3_DLL spica::Vector3 operator/(const spica::Vector3& v, double s);

SPICA_VECTOR3_DLL std::ostream& operator<<(std::ostream& os, const spica::Vector3 v);

#endif  // SPICA_VECTOR3_H_
