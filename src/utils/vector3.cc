#define SPICA_VECTOR3_EXPORT
#include "vector3.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include "common.h"

namespace spica {

    Vector3::Vector3()
        : _xyz()
    {
    }

    Vector3::Vector3(double x, double y, double z)
        : _xyz()
    {
        _xyz[0] = x;
        _xyz[1] = y;
        _xyz[2] = z;
    }

    Vector3::Vector3(const Vector3& v)
        : _xyz(v._xyz)
    {
    }

    Vector3::~Vector3() {
    }

    Vector3& Vector3::operator=(const Vector3& v) {
        this->_xyz = v._xyz;
        return *this;
    }

    Vector3& Vector3::operator+=(const Vector3& v) {
        this->x() += v.x();
        this->y() += v.y();
        this->z() += v.z();
        return *this;
    }

    Vector3& Vector3::operator-=(const Vector3& v) {
        this->operator+=(-v);
        return *this;
    }

    Vector3 Vector3::operator-() const {
        return Vector3(-this->x(), -this->y(), -this->z());
    }

    Vector3& Vector3::operator*=(double s) {
        this->x() *= s;
        this->y() *= s;
        this->z() *= s;
        return *this;
    }

    Vector3& Vector3::operator/=(double s) {
        msg_assert(s != 0.0, "Zero division");
        return this->operator*=(1.0 / s);
    }

    bool Vector3::operator==(const Vector3& v) const {
        return (this->x() == v.x() && this->y() == v.y() && this->z() == v.z());
    }

    double Vector3::dot(const Vector3& v) const {
        return this->x() * v.x() + this->y() * v.y() + this->z() * v.z();
    }

    double Vector3::dot(const Vector3& v1, const Vector3& v2) {
        return v1.dot(v2);
    }

    Vector3 Vector3::cross(const Vector3& v) const {
        double x = this->y() * v.z() - this->z() * v.y();
        double y = this->z() * v.x() - this->x() * v.z();
        double z = this->x() * v.y() - this->y() * v.x();
        return Vector3(x, y, z);
    }

    Vector3 Vector3::cross(const Vector3& v1, const Vector3& v2) {
        return v1.cross(v2);
    }

    double Vector3::norm() const {
        return sqrt(this->squaredNorm());
    }

    double Vector3::squaredNorm() const {
        return this->dot(*this);
    }

    Vector3 Vector3::normalized() const {
        Vector3 ret = *this;
        ret /= ret.norm();
        return ret;
    }

    Vector3 Vector3::normalize(const Vector3& v) {
        return v.normalized();
    }

    Vector3 Vector3::multiply(const Vector3& v) const {
        return Vector3(this->x() * v.x(), this->y() * v.y(), this->z() * v.z());
    }

    Vector3 Vector3::minimum(const Vector3& v1, const Vector3& v2) {
        double x = std::min(v1.x(), v2.x());
        double y = std::min(v1.y(), v2.y());
        double z = std::min(v1.z(), v2.z());
        return Vector3(x, y, z);
    }

    Vector3 Vector3::maximum(const Vector3& v1, const Vector3& v2) {
        double x = std::max(v1.x(), v2.x());
        double y = std::max(v1.y(), v2.y());
        double z = std::max(v1.z(), v2.z());
        return Vector3(x, y, z);
    }


    Vector3 Vector3::reflect(const Vector3& v, const Vector3& n) {
        return (v - n * 2.0 * n.dot(v));
    }

    double Vector3::get(int d) const {
        msg_assert(0 <= d && d <= 2, "Dimension must be between 0 and 2");
        return _xyz[d];
    }

    std::string Vector3::toString() const {
        std::stringstream ss;
        ss << "(" << _xyz[0] << ", " << _xyz[1] << ", " << _xyz[2] << ")";
        return ss.str();
    }
}

spica::Vector3 operator+(const spica::Vector3& v1, const spica::Vector3& v2) {
    spica::Vector3 ret = v1;
    ret += v2;
    return ret;
}

spica::Vector3 operator-(const spica::Vector3& v1, const spica::Vector3& v2) {
    spica::Vector3 ret = v1;
    ret -= v2;
    return ret;
}

spica::Vector3 operator*(const spica::Vector3& v, double s) {
    spica::Vector3 ret = v;
    ret *= s;
    return ret;
}

spica::Vector3 operator*(double s, const spica::Vector3& v) {
    spica::Vector3 ret = v;
    ret *= s;
    return ret;
}

spica::Vector3 operator/(const spica::Vector3& v, double s) {
    spica::Vector3 ret = v;
    ret /= s;
    return ret;
}

std::ostream& operator<<(std::ostream& os, const spica::Vector3 v) {
    os << v.toString();
    return os;
}
