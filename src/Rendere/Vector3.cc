#define RAINY_VECTOR3_EXPORT
#include "Vector3.h"
#include "common.h"

namespace rainy {

    Vector3::Vector3()
        : _x(0.0)
        , _y(0.0)
        , _z(0.0) {
    }

    Vector3::Vector3(double x, double y, double z)
        : _x(x)
        , _y(y)
        , _z(z) {
    }

    Vector3::Vector3(const Vector3& v)
        : _x(v._x)
        , _y(v._y)
        , _z(v._z) {
    }

    Vector3::~Vector3() {
    }

    Vector3& Vector3::operator=(const Vector3& v) {
        this->_x = v._x;
        this->_y = v._y;
        this->_z = v._z;
        return *this;
    }

    Vector3& Vector3::operator+=(const Vector3& v) {
        this->_x += v._x;
        this->_y += v._y;
        this->_z += v._z;
        return *this;
    }

    Vector3& Vector3::operator-=(const Vector3& v) {
        this->operator+=(-v);
        return *this;
    }

    Vector3 Vector3::operator-() const {
        return Vector3(-_x, -_y, -_z);
    }

    Vector3& Vector3::operator*=(double s) {
        this->_x *= s;
        this->_y *= s;
        this->_z *= s;
        return *this;
    }

    Vector3& Vector3::operator/=(double s) {
        msg_assert(s != 0.0, "Zero division");
        this->_x /= s;
        this->_y /= s;
        this->_z /= s;
        return *this;
    }

    double Vector3::dot(const Vector3& v) const {
        return this->_x * v._x + this->_y * v._y + this->_z * v._z;
    }

    Vector3 Vector3::cross(const Vector3& v) const {
        double x = this->_y * v._z - this->_z * v._y;
        double y = this->_z * v._x - this->_x * v._z;
        double z = this->_x * v._y - this->_y * v._x;
        return Vector3(x, y, z);
    }

    double Vector3::norm() const {
        return sqrt(this->dot(*this));
    }

    Vector3 Vector3::normalize() const {
        Vector3 ret = *this;
        ret /= ret.norm();
        return ret;
    }

    Vector3 Vector3::cwiseMultiply(const Vector3& v) const {
        return Vector3(this->_x * v._x, this->_y * v._y, this->_z * v._z);
    }

    double Vector3::x() const {
        return _x;
    }

    double Vector3::y() const {
        return _y;
    }

    double Vector3::z() const {
        return _z;
    }
}

rainy::Vector3 operator+(const rainy::Vector3& v1, const rainy::Vector3& v2) {
    rainy::Vector3 ret = v1;
    ret += v2;
    return ret;
}

rainy::Vector3 operator-(const rainy::Vector3& v1, const rainy::Vector3& v2) {
    rainy::Vector3 ret = v1;
    ret -= v2;
    return ret;
}

rainy::Vector3 operator*(const rainy::Vector3& v, double s) {
    rainy::Vector3 ret = v;
    ret *= s;
    return ret;
}

rainy::Vector3 operator*(double s, const rainy::Vector3& v) {
    rainy::Vector3 ret = v;
    ret *= s;
    return ret;
}

rainy::Vector3 operator/(const rainy::Vector3& v, double s) {
    rainy::Vector3 ret = v;
    ret /= s;
    return ret;
}
