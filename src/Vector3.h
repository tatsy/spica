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
        Vector3& operator*=(double s);
        Vector3& operator/=(double s);

        double dot(const Vector3& v) const;
        Vector3 cross(const Vector3& v) const;

        double x() const;
        double y() const;
        double z() const;
    };  // class Vector3

}  // namespace rainy

#endif  // RAINY_VECTOR3_H_
