#define SPICA_VECTOR3D_EXPORT
#include "vector3d.h"

#ifndef __AVX__

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "common.h"

namespace spica {

    Vector3D::Vector3D()
        : _xyz()
    {
    }

    Vector3D::Vector3D(double x, double y, double z)
        : _xyz()
    {
        _xyz[0] = x;
        _xyz[1] = y;
        _xyz[2] = z;
    }

    Vector3D::Vector3D(const Vector3D& v)
        : _xyz(v._xyz)
    {
    }

    Vector3D::~Vector3D() {
    }

    Vector3D& Vector3D::operator=(const Vector3D& v) {
        this->_xyz = v._xyz;
        return *this;
    }

    Vector3D& Vector3D::operator+=(const Vector3D& v) {
        this->x() += v.x();
        this->y() += v.y();
        this->z() += v.z();
        return *this;
    }

    Vector3D& Vector3D::operator+=(double x) {
        this->x() += x;
        this->y() += x;
        this->z() += x;
        return *this;
    }

    Vector3D& Vector3D::operator-=(const Vector3D& v) {
        this->operator+=(-v);
        return *this;
    }

    Vector3D& Vector3D::operator-=(double x) {
        this->operator+=(-x);
        return *this;
    }

    Vector3D Vector3D::operator-() const {
        return Vector3D(-this->x(), -this->y(), -this->z());
    }

    Vector3D& Vector3D::operator*=(const Vector3D& v) {
        this->x() *= v.x();
        this->y() *= v.y();
        this->z() *= v.z();
        return *this;
    }

    Vector3D& Vector3D::operator*=(double s) {
        this->x() *= s;
        this->y() *= s;
        this->z() *= s;
        return *this;
    }

    Vector3D& Vector3D::operator/=(const Vector3D& v) {
        Assertion(v.x() != 0.0 && v.y() != 0.0 && v.z() != 0.0, "Zero division");
        this->x() /= v.x();
        this->y() /= v.y();
        this->z() /= v.z();
        return *this;
    }

    Vector3D& Vector3D::operator/=(double s) {
        Assertion(s != 0.0, "Zero division");
        return this->operator*=(1.0 / s);
    }

    bool Vector3D::operator==(const Vector3D& v) const {
        return (this->x() == v.x() && this->y() == v.y() && this->z() == v.z());
    }

    double Vector3D::dot(const Vector3D& v) const {
        return this->x() * v.x() + this->y() * v.y() + this->z() * v.z();
    }

    double Vector3D::dot(const Vector3D& v1, const Vector3D& v2) {
        return v1.dot(v2);
    }

    Vector3D Vector3D::cross(const Vector3D& v) const {
        double x = this->y() * v.z() - this->z() * v.y();
        double y = this->z() * v.x() - this->x() * v.z();
        double z = this->x() * v.y() - this->y() * v.x();
        return Vector3D(x, y, z);
    }

    Vector3D Vector3D::cross(const Vector3D& v1, const Vector3D& v2) {
        return v1.cross(v2);
    }

    Vector3D Vector3D::sqrt(const Vector3D& v) {
        Assertion(v.x() >= 0.0 && v.y() >= 0.0 && v.z() >= 0.0, "Specified vector has negative entries !!");
        return Vector3D(::sqrt(v.x()), ::sqrt(v.y()), ::sqrt(v.z()));
    }

    Vector3D Vector3D::exp(const Vector3D& v) {
        return Vector3D(::exp(v.x()), ::exp(v.y()), ::exp(v.z()));
    }

    double Vector3D::norm() const {
        return ::sqrt(this->squaredNorm());
    }

    double Vector3D::squaredNorm() const {
        return this->dot(*this);
    }

    Vector3D Vector3D::normalized() const {
        Vector3D ret = *this;
        ret /= ret.norm();
        return ret;
    }

    Vector3D Vector3D::normalize(const Vector3D& v) {
        return v.normalized();
    }

    Vector3D Vector3D::multiply(const Vector3D& v) const {
        return Vector3D(this->x() * v.x(), this->y() * v.y(), this->z() * v.z());
    }

    Vector3D Vector3D::minimum(const Vector3D& v1, const Vector3D& v2) {
        double x = std::min(v1.x(), v2.x());
        double y = std::min(v1.y(), v2.y());
        double z = std::min(v1.z(), v2.z());
        return Vector3D(x, y, z);
    }

    Vector3D Vector3D::maximum(const Vector3D& v1, const Vector3D& v2) {
        double x = std::max(v1.x(), v2.x());
        double y = std::max(v1.y(), v2.y());
        double z = std::max(v1.z(), v2.z());
        return Vector3D(x, y, z);
    }


    Vector3D Vector3D::reflect(const Vector3D& v, const Vector3D& n) {
        return (v - n * 2.0 * n.dot(v));
    }

    double Vector3D::get(int d) const {
        Assertion(0 <= d && d <= 2, "Dimension must be between 0 and 2");
        return _xyz[d];
    }

    std::string Vector3D::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(8);
        ss << "(" << _xyz[0] << ", " << _xyz[1] << ", " << _xyz[2] << ")";
        return ss.str();
    }

    double Vector3D::x() const { return _xyz[0]; }
    double Vector3D::y() const { return _xyz[1]; }
    double Vector3D::z() const { return _xyz[2]; }
    double& Vector3D::x() { return _xyz[0]; }
    double& Vector3D::y() { return _xyz[1]; }
    double& Vector3D::z() { return _xyz[2]; }
}

spica::Vector3D operator+(const spica::Vector3D& v1, const spica::Vector3D& v2) {
    spica::Vector3D ret = v1;
    ret += v2;
    return ret;
}

spica::Vector3D operator-(const spica::Vector3D& v1, const spica::Vector3D& v2) {
    spica::Vector3D ret = v1;
    ret -= v2;
    return ret;
}

spica::Vector3D operator+(const spica::Vector3D& v, double x) {
    spica::Vector3D ret = v;
    ret += x;
    return ret;
}

spica::Vector3D operator+(double x, const spica::Vector3D& v) {
    return v + x;
}

spica::Vector3D operator-(const spica::Vector3D& v, double x) {
    return v + (-x);
}

spica::Vector3D operator-(double x, const spica::Vector3D& v) {
    return v + (-x);
}

spica::Vector3D operator*(const spica::Vector3D& u, const spica::Vector3D& v) {
    spica::Vector3D ret = u;
    ret *= v;
    return ret;
}

spica::Vector3D operator*(const spica::Vector3D& v, double s) {
    spica::Vector3D ret = v;
    ret *= s;
    return ret;
}

spica::Vector3D operator*(double s, const spica::Vector3D& v) {
    spica::Vector3D ret = v;
    ret *= s;
    return ret;
}

spica::Vector3D operator/(const spica::Vector3D& u, const spica::Vector3D& v) {
    spica::Vector3D ret = u;
    ret /= v;
    return ret;
}

spica::Vector3D operator/(const spica::Vector3D& v, double s) {
    spica::Vector3D ret = v;
    ret /= s;
    return ret;
}

std::ostream& operator<<(std::ostream& os, const spica::Vector3D v) {
    os << v.toString();
    return os;
}

#endif  // ENABLE_EVX