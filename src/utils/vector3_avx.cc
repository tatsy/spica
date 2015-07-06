#define SPICA_VECTOR3_EXPORT
#include "vector3.h"

#ifdef __AVX__

#include <iostream>
#include <sstream>
#include <algorithm>

namespace spica {

    Vector3::Vector3()
        : _xyz()
    {
        _xyz.m = _mm256_setr_pd(0.0, 0.0, 0.0, 0.0);
    }

    Vector3::Vector3(double x, double y, double z)
        : _xyz()
    {
        _xyz.m = _mm256_setr_pd(x, y, z, 0.0);
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
        __m256d ss   = _mm256_setr_pd(s, s, s, 0.0);
        this->_xyz.m = _mm256_mul_pd(this->_xyz.m, ss);
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
        m256d tmp = { _mm256_mul_pd(_xyz.m, v._xyz.m) };
        return tmp.v[0] + tmp.v[1] + tmp.v[2];
    }

    double Vector3::dot(const Vector3& v1, const Vector3& v2) {
        return v1.dot(v2);
    }

    Vector3 Vector3::cross(const Vector3& v) const {
        __m256d a_yzx = _mm256_setr_pd(_xyz.v[1], _xyz.v[2], _xyz.v[0], 0.0);
        __m256d a_zxy = _mm256_setr_pd(_xyz.v[2], _xyz.v[0], _xyz.v[1], 0.0);
        __m256d b_yzx = _mm256_setr_pd(v._xyz.v[1], v._xyz.v[2], v._xyz.v[0], 0.0);
        __m256d b_zxy = _mm256_setr_pd(v._xyz.v[2], v._xyz.v[0], v._xyz.v[1], 0.0);

        Vector3 ret;
        __m256d pos = _mm256_mul_pd(a_yzx, b_zxy);
        __m256d neg = _mm256_mul_pd(a_zxy, b_yzx);
        ret._xyz.m = _mm256_sub_pd(pos, neg);
        return ret;
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
        Vector3 ret;
        ret._xyz.m = _mm256_mul_pd(_xyz.m, v._xyz.m);
        return ret;
    }

    Vector3 Vector3::minimum(const Vector3& v1, const Vector3& v2) {
        Vector3 ret;
        ret._xyz.m = _mm256_min_pd(v1._xyz.m, v2._xyz.m);
        return ret;
    }

    Vector3 Vector3::maximum(const Vector3& v1, const Vector3& v2) {
        Vector3 ret;
        ret._xyz.m = _mm256_max_pd(v1._xyz.m, v2._xyz.m);
        return ret;
    }


    Vector3 Vector3::reflect(const Vector3& v, const Vector3& n) {
        return (v - n * (2.0 * n.dot(v)));
    }

    double Vector3::get(int d) const {
        msg_assert(0 <= d && d <= 2, "Dimension must be between 0 and 2");
        return _xyz.v[d];
    }

    std::string Vector3::toString() const {
        std::stringstream ss;
        ss << "(" << _xyz.v[0] << ", " << _xyz.v[1] << ", " << _xyz.v[2] << ")";
        return ss.str();
    }

    double Vector3::x() const { return _xyz.v[0]; }
    double Vector3::y() const { return _xyz.v[1]; }
    double Vector3::z() const { return _xyz.v[2]; }
    double& Vector3::x() { return _xyz.v[0]; }
    double& Vector3::y() { return _xyz.v[1]; }
    double& Vector3::z() { return _xyz.v[2]; }
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

#endif  // ENABLE_AVX