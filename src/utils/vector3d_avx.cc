#define SPICA_VECTOR3_EXPORT
#include "vector3d.h"

#ifdef __AVX__

#include <iostream>
#include <sstream>
#include <algorithm>

namespace spica {

    Vector3D::Vector3D()
        : _xyz()
    {
        _xyz.m = _mm256_setr_pd(0.0, 0.0, 0.0, 0.0);
    }

    Vector3D::Vector3D(double x, double y, double z)
        : _xyz()
    {
        _xyz.m = _mm256_setr_pd(x, y, z, 0.0);
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

    Vector3D& Vector3D::operator-=(const Vector3D& v) {
        this->operator+=(-v);
        return *this;
    }

    Vector3D Vector3D::operator-() const {
        return Vector3D(-this->x(), -this->y(), -this->z());
    }

    Vector3D& Vector3D::operator*=(double s) {
        __m256d ss   = _mm256_setr_pd(s, s, s, 0.0);
        this->_xyz.m = _mm256_mul_pd(this->_xyz.m, ss);
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
        m256d tmp = { _mm256_mul_pd(_xyz.m, v._xyz.m) };
        return tmp.v[0] + tmp.v[1] + tmp.v[2];
    }

    double Vector3D::dot(const Vector3D& v1, const Vector3D& v2) {
        return v1.dot(v2);
    }

    Vector3D Vector3D::cross(const Vector3D& v) const {
        __m256d a_yzx = _mm256_setr_pd(_xyz.v[1], _xyz.v[2], _xyz.v[0], 0.0);
        __m256d a_zxy = _mm256_setr_pd(_xyz.v[2], _xyz.v[0], _xyz.v[1], 0.0);
        __m256d b_yzx = _mm256_setr_pd(v._xyz.v[1], v._xyz.v[2], v._xyz.v[0], 0.0);
        __m256d b_zxy = _mm256_setr_pd(v._xyz.v[2], v._xyz.v[0], v._xyz.v[1], 0.0);

        Vector3D ret;
        __m256d pos = _mm256_mul_pd(a_yzx, b_zxy);
        __m256d neg = _mm256_mul_pd(a_zxy, b_yzx);
        ret._xyz.m = _mm256_sub_pd(pos, neg);
        return ret;
    }

    Vector3D Vector3D::cross(const Vector3D& v1, const Vector3D& v2) {
        return v1.cross(v2);
    }

    double Vector3D::norm() const {
        return sqrt(this->squaredNorm());
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
        Vector3D ret;
        ret._xyz.m = _mm256_mul_pd(_xyz.m, v._xyz.m);
        return ret;
    }

    Vector3D Vector3D::minimum(const Vector3D& v1, const Vector3D& v2) {
        Vector3D ret;
        ret._xyz.m = _mm256_min_pd(v1._xyz.m, v2._xyz.m);
        return ret;
    }

    Vector3D Vector3D::maximum(const Vector3D& v1, const Vector3D& v2) {
        Vector3D ret;
        ret._xyz.m = _mm256_max_pd(v1._xyz.m, v2._xyz.m);
        return ret;
    }


    Vector3D Vector3D::reflect(const Vector3D& v, const Vector3D& n) {
        return (v - n * (2.0 * n.dot(v)));
    }

    double Vector3D::get(int d) const {
        Assertion(0 <= d && d <= 2, "Dimension must be between 0 and 2");
        return _xyz.v[d];
    }

    std::string Vector3D::toString() const {
        std::stringstream ss;
        ss << "(" << _xyz.v[0] << ", " << _xyz.v[1] << ", " << _xyz.v[2] << ")";
        return ss.str();
    }

    double Vector3D::x() const { return _xyz.v[0]; }
    double Vector3D::y() const { return _xyz.v[1]; }
    double Vector3D::z() const { return _xyz.v[2]; }
    double& Vector3D::x() { return _xyz.v[0]; }
    double& Vector3D::y() { return _xyz.v[1]; }
    double& Vector3D::z() { return _xyz.v[2]; }
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

spica::Vector3D operator/(const spica::Vector3D& v, double s) {
    spica::Vector3D ret = v;
    ret /= s;
    return ret;
}

std::ostream& operator<<(std::ostream& os, const spica::Vector3D v) {
    os << v.toString();
    return os;
}

#endif  // ENABLE_AVX