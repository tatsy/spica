#ifndef RAINY_VECTOR3_H_
#define RAINY_VECTOR3_H_

namespace rainy {

    class Vector3 {
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

}  // namespace rainy

rainy::Vector3 operator+(const rainy::Vector3& v1, const rainy::Vector3& v2);
rainy::Vector3 operator-(const rainy::Vector3& v1, const rainy::Vector3& v2);
rainy::Vector3 operator*(const rainy::Vector3& v, double s);
rainy::Vector3 operator*(double s, const rainy::Vector3& v);
rainy::Vector3 operator/(const rainy::Vector3& v, double s);

#endif  // RAINY_VECTOR3_H_
