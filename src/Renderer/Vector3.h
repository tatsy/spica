#ifndef RAINY_VECTOR3_H_
#define RAINY_VECTOR3_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef RAINY_VECTOR3_EXPORT
        #define RAINY_VECTOR3_DLL __declspec(dllexport)
    #else
        #define RAINY_VECTOR3_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define RAINY_VECTOR3_DLL
#endif

namespace spica {

    class RAINY_VECTOR3_DLL Vector3 {
    private:
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
    };  // class Vector3

}  // namespace spica

RAINY_VECTOR3_DLL spica::Vector3 operator+(const spica::Vector3& v1, const spica::Vector3& v2);
RAINY_VECTOR3_DLL spica::Vector3 operator-(const spica::Vector3& v1, const spica::Vector3& v2);
RAINY_VECTOR3_DLL spica::Vector3 operator*(const spica::Vector3& v, double s);
RAINY_VECTOR3_DLL spica::Vector3 operator*(double s, const spica::Vector3& v);
RAINY_VECTOR3_DLL spica::Vector3 operator/(const spica::Vector3& v, double s);

#endif  // RAINY_VECTOR3_H_
