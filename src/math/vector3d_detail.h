#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECTOR3D_DETAIL_H_
#define _SPICA_VECTOR3D_DETAIL_H_

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace spica {

    template <class T>
    Vector3_<T, is_arith_t<T> >::Vector3_()
        : _x{0}
        , _y{0}
        , _z{0} {
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >::Vector3_(T x, T y, T z)
        : _x{x}
        , _y{y}
        , _z{z} {
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >::Vector3_(const Vector3_<T, is_arith_t<T> >& v)
        : _x{v._x}
        , _y{v._y}
        , _z{v._z} {
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >::~Vector3_() {
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >&
    Vector3_<T, is_arith_t<T> >::operator=(const Vector3_<T, is_arith_t<T> >& v) {
        this->_x = v._x;
        this->_y = v._y;
        this->_z = v._z;
        return *this;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >&
    Vector3_<T, is_arith_t<T> >::operator+=(const Vector3_<T, is_arith_t<T> >& v) {
        this->_x += v._x;
        this->_y += v._y;
        this->_z += v._z;
        return *this;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >&
    Vector3_<T, is_arith_t<T> >::operator+=(double x) {
        this->_x += x;
        this->_y += x;
        this->_z += x;
        return *this;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >&
    Vector3_<T, is_arith_t<T> >::operator-=(const Vector3_<T, is_arith_t<T> >& v) {
        this->operator+=(-v);
        return *this;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >&
    Vector3_<T, is_arith_t<T> >::operator-=(double x) {
        this->operator+=(-x);
        return *this;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >
    Vector3_<T, is_arith_t<T> >::operator-() const {
        return { -_x, -_y, -_z };
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >&
    Vector3_<T, is_arith_t<T> >::operator*=(const Vector3_<T, is_arith_t<T> >& v) {
        this->_x *= v._x;
        this->_y *= v._y;
        this->_z *= v._z;
        return *this;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >&
    Vector3_<T, is_arith_t<T> >::operator*=(double s) {
        this->_x *= s;
        this->_y *= s;
        this->_z *= s;
        return *this;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >&
    Vector3_<T, is_arith_t<T> >::operator/=(const Vector3_<T, is_arith_t<T> >& v) {
        Assertion(v._x != 0 && v._y != 0 && v._z != 0, "Zero division");
        this->_x /= v._x;
        this->_y /= v._y;
        this->_z /= v._z;
        return *this;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >&
    Vector3_<T, is_arith_t<T> >::operator/=(double s) {
        Assertion(s != 0.0, "Zero division");
        return this->operator*=(1.0 / s);
    }

    template <class T>
    bool Vector3_<T, is_arith_t<T> >::operator==(const Vector3_<T, is_arith_t<T> >& v) const {
        return (this->_x == v._x && this->_y == v._y && this->_z == v._z);
    }

    template <class T>
    bool Vector3_<T, is_arith_t<T> >::operator!=(const Vector3_<T, is_arith_t<T> >& v) const {
        return !this->operator==(v);
    }

    template <class T>
    T Vector3_<T, is_arith_t<T> >::dot(const Vector3_<T, is_arith_t<T> >& v) const {
        return this->_x * v._x + this->_y * v._y + this->_z * v._z;
    }

    template <class T>
    T Vector3_<T, is_arith_t<T> >::dot(const Vector3_<T, is_arith_t<T> >& v1,
                                       const Vector3_<T, is_arith_t<T> >& v2) {
        return v1.dot(v2);
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >
    Vector3_<T, is_arith_t<T> >::cross(const Vector3_<T, is_arith_t<T> >& v) const {
        T x = this->_y * v._z - this->_z * v._y;
        T y = this->_z * v._x - this->_x * v._z;
        T z = this->_x * v._y - this->_y * v._x;
        return { x, y, z };
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >
    Vector3_<T, is_arith_t<T> >::cross(const Vector3_<T, is_arith_t<T> >& v1,
                                       const Vector3_<T, is_arith_t<T> >& v2) {
        return v1.cross(v2);
    }

    template <class T>
    double Vector3_<T, is_arith_t<T> >::norm() const {
        return ::sqrt(this->squaredNorm());
    }

    template <class T>
    double Vector3_<T, is_arith_t<T> >::squaredNorm() const {
        return this->dot(*this);
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >
    Vector3_<T, is_arith_t<T> >::normalized() const {
        Vector3_<T, is_arith_t<T> > ret = *this;
        ret /= ret.norm();
        return ret;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >
    Vector3_<T, is_arith_t<T> >::normalize(const Vector3_<T, is_arith_t<T> >& v) {
        return v.normalized();
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >
    Vector3_<T, is_arith_t<T> >::multiply(const Vector3_<T, is_arith_t<T> >& v) const {
        return (*this) * v;
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >
    Vector3_<T, is_arith_t<T> >::minimum(const Vector3_<T, is_arith_t<T> >& v1,
                                         const Vector3_<T, is_arith_t<T> >& v2) {
        T x = std::min(v1._x, v2._x);
        T y = std::min(v1._y, v2._y);
        T z = std::min(v1._z, v2._z);
        return { x, y, z };
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >
    Vector3_<T, is_arith_t<T> >::maximum(const Vector3_<T, is_arith_t<T> >& v1,
                                         const Vector3_<T, is_arith_t<T> >& v2) {
        T x = std::max(v1.x(), v2.x());
        T y = std::max(v1.y(), v2.y());
        T z = std::max(v1.z(), v2.z());
        return { x, y, z };
    }

    template <class T>
    Vector3_<T, is_arith_t<T> >
    Vector3_<T, is_arith_t<T> >::reflect(const Vector3_<T, is_arith_t<T> >& v,
                                         const Vector3_<T, is_arith_t<T> >& n) {
        return (v - n * T(2) * n.dot(v));
    }

    template <class T>
    T Vector3_<T, is_arith_t<T> >::get(int d) const {
        Assertion(0 <= d && d <= 2, "Dimension must be between 0 and 2");
        if (d == 0) return _x;
        if (d == 1) return _y;
        if (d == 2) return _z;
        return 0.0;
    }

    template <class T>
    std::string Vector3_<T, is_arith_t<T> >::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(8);
        ss << "(" << _x << ", " << _y << ", " << _z << ")";
        return std::move(ss.str());
    }

}  // namespace spica

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator+(const spica::Vector3_<T, is_arith_t<T> >& v1,
          const spica::Vector3_<T, is_arith_t<T> >& v2) {
    spica::Vector3_<T, is_arith_t<T> > ret = v1;
    ret += v2;
    return ret;
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator-(const spica::Vector3_<T, is_arith_t<T> >& v1,
          const spica::Vector3_<T, is_arith_t<T> >& v2) {
    spica::Vector3_<T, is_arith_t<T> > ret = v1;
    ret -= v2;
    return ret;
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator+(const spica::Vector3_<T, is_arith_t<T> >& v, T x) {
    spica::Vector3_<T, is_arith_t<T> > ret = v;
    ret += x;
    return ret;
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator+(T x, const spica::Vector3_<T, is_arith_t<T> >& v) {
    return v + x;
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator-(const spica::Vector3_<T, is_arith_t<T> >& v, T x) {
    return v + (-x);
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator-(T x, const spica::Vector3_<T, is_arith_t<T> >& v) {
    return x + (-v);
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator*(const spica::Vector3_<T, is_arith_t<T> >& u,
          const spica::Vector3_<T, is_arith_t<T> >& v) {
    spica::Vector3_<T, is_arith_t<T> > ret = u;
    ret *= v;
    return ret;
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator*(const spica::Vector3_<T, is_arith_t<T> >& v, double s) {
    spica::Vector3_<T, is_arith_t<T> > ret = v;
    ret *= s;
    return ret;
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator*(double s, const spica::Vector3_<T, is_arith_t<T> >& v) {
    spica::Vector3_<T, is_arith_t<T> > ret = v;
    ret *= s;
    return ret;
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator/(const spica::Vector3_<T, is_arith_t<T> >& u,
          const spica::Vector3_<T, is_arith_t<T> >& v) {
    spica::Vector3_<T, is_arith_t<T> > ret = u;
    ret /= v;
    return ret;
}

template <class T>
spica::Vector3_<T, is_arith_t<T> >
operator/(const spica::Vector3_<T, is_arith_t<T> >& v, double s) {
    spica::Vector3_<T, is_arith_t<T> > ret = v;
    ret /= s;
    return ret;
}

template <class T>
std::ostream& operator<<(std::ostream& os,
                         const spica::Vector3_<T, is_arith_t<T> >& v) {
    os << v.toString();
    return os;
}

#endif  // _SPICA_VECTOR3D_DETAIL_H_
