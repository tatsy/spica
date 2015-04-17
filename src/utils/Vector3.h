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

namespace spica {

    class SPICA_VECTOR3_DLL Vector3 {
    protected:
        double _x, _y, _z;

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

        double dot(const Vector3& v) const;
        Vector3 cross(const Vector3& v) const;
        double norm() const;
        Vector3 normalize() const;

        Vector3 cwiseMultiply(const Vector3& v) const;

        double x() const;
        double y() const;
        double z() const;

        inline void setX(double x) { _x = x; }
        inline void setY(double y) { _y = y; }
        inline void setZ(double z) { _z = z; }

    };  // class Vector3

}  // namespace spica

SPICA_VECTOR3_DLL spica::Vector3 operator+(const spica::Vector3& v1, const spica::Vector3& v2);
SPICA_VECTOR3_DLL spica::Vector3 operator-(const spica::Vector3& v1, const spica::Vector3& v2);
SPICA_VECTOR3_DLL spica::Vector3 operator*(const spica::Vector3& v, double s);
SPICA_VECTOR3_DLL spica::Vector3 operator*(double s, const spica::Vector3& v);
SPICA_VECTOR3_DLL spica::Vector3 operator/(const spica::Vector3& v, double s);

SPICA_VECTOR3_DLL std::ostream& operator<<(std::ostream& os, const spica::Vector3& v);

#endif  // SPICA_VECTOR3_H_
