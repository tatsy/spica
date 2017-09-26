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
    Vector3_<T>::Vector3_()
        : x_{ 0 }
        , y_{ 0 }
        , z_{ 0 } {
    }

    template <class T>
    Vector3_<T>::Vector3_(T x, T y, T z)
        : x_{x}
        , y_{y}
        , z_{z} {
    }

    template <class T>
    Vector3_<T>::Vector3_(const std::string &str) {
        double x, y, z;
        if (sscanf(str.c_str(), "%lf %lf %lf", &x ,&y, &z) == 3 ||
            sscanf(str.c_str(), "%lf, %lf, %lf", &x, &y, &z) == 3) {
            this->x_ = x;
            this->y_ = y;
            this->z_ = z;
        } else if (sscanf(str.c_str(), "%lf", &x) == 1) {
            this->x_ = x;
            this->y_ = x;
            this->z_ = x;
        } else {
            FatalError("Cannot parse string \"%s\" for Vector3d", str.c_str());
        }
    }

    template <class T>
    Vector3_<T>::Vector3_(const Vector3_<T>& v)
        : x_{v.x_}
        , y_{v.y_}
        , z_{v.z_} {
    }

    template <class T>
    Vector3_<T>::~Vector3_() {
    }

    template <class T>
    Vector3_<T>&
    Vector3_<T>::operator=(const Vector3_<T>& v) {
        this->x_ = v.x_;
        this->y_ = v.y_;
        this->z_ = v.z_;
        return *this;
    }

    template <class T>
    Vector3_<T>&
    Vector3_<T>::operator+=(const Vector3_<T>& v) {
        this->x_ += v.x_;
        this->y_ += v.y_;
        this->z_ += v.z_;
        return *this;
    }

    template <class T>
    Vector3_<T>&
    Vector3_<T>::operator+=(double x) {
        this->x_ += x;
        this->y_ += x;
        this->z_ += x;
        return *this;
    }

    template <class T>
    Vector3_<T>&
    Vector3_<T>::operator-=(const Vector3_<T>& v) {
        this->operator+=(-v);
        return *this;
    }

    template <class T>
    Vector3_<T>&
    Vector3_<T>::operator-=(double x) {
        this->operator+=(-x);
        return *this;
    }

    template <class T>
    Vector3_<T>
    Vector3_<T>::operator-() const {
        return { -x_, -y_, -z_ };
    }

    template <class T>
    Vector3_<T>&
    Vector3_<T>::operator*=(const Vector3_<T>& v) {
        this->x_ *= v.x_;
        this->y_ *= v.y_;
        this->z_ *= v.z_;
        return *this;
    }

    template <class T>
    Vector3_<T>&
    Vector3_<T>::operator*=(double s) {
        this->x_ *= s;
        this->y_ *= s;
        this->z_ *= s;
        return *this;
    }

    template <class T>
    Vector3_<T>&
    Vector3_<T>::operator/=(const Vector3_<T>& v) {
        Assertion(v.x_ != 0 && v.y_ != 0 && v.z_ != 0, "Zero division");
        this->x_ /= v.x_;
        this->y_ /= v.y_;
        this->z_ /= v.z_;
        return *this;
    }

    template <class T>
    Vector3_<T>&
    Vector3_<T>::operator/=(double s) {
        Assertion(s != 0.0, "Zero division");
        return this->operator*=(1.0 / s);
    }

    template <class T>
    T Vector3_<T>::operator[](int i) const {
        Assertion(i >= 0 && i <= 2, "Index out of bounds!!");
        if (i == 0) return x_;
        if (i == 1) return y_;
        return z_;
    }

    template <class T>
    bool Vector3_<T>::operator==(const Vector3_<T>& v) const {
        return (this->x_ == v.x_ && this->y_ == v.y_ && this->z_ == v.z_);
    }

    template <class T>
    bool Vector3_<T>::operator!=(const Vector3_<T>& v) const {
        return !this->operator==(v);
    }

    template <class T>
    T Vector3_<T>::dot(const Vector3_<T>& v) const {
        return this->x_ * v.x_ + this->y_ * v.y_ + this->z_ * v.z_;
    }

    template <class T>
    T Vector3_<T>::dot(const Vector3_<T>& v1,
                                       const Vector3_<T>& v2) {
        return v1.dot(v2);
    }

    template <class T>
    Vector3_<T>
    Vector3_<T>::cross(const Vector3_<T>& v) const {
        T x = this->y_ * v.z_ - this->z_ * v.y_;
        T y = this->z_ * v.x_ - this->x_ * v.z_;
        T z = this->x_ * v.y_ - this->y_ * v.x_;
        return { x, y, z };
    }

    template <class T>
    Vector3_<T>
    Vector3_<T>::cross(const Vector3_<T>& v1,
                                       const Vector3_<T>& v2) {
        return v1.cross(v2);
    }

    template <class T>
    double Vector3_<T>::norm() const {
        return ::sqrt(this->squaredNorm());
    }

    template <class T>
    double Vector3_<T>::squaredNorm() const {
        return this->dot(*this);
    }

    template <class T>
    Vector3_<T>
    Vector3_<T>::normalized() const {
        Vector3_<T> ret = *this;
        ret /= ret.norm();
        return ret;
    }

    template <class T>
    Vector3_<T>
    Vector3_<T>::normalize(const Vector3_<T>& v) {
        return v.normalized();
    }

    template <class T>
    Vector3_<T>
    Vector3_<T>::multiply(const Vector3_<T>& v) const {
        return (*this) * v;
    }

    template <class T>
    Vector3_<T>
    Vector3_<T>::minimum(const Vector3_<T>& v1,
                                         const Vector3_<T>& v2) {
        T x = std::min(v1.x_, v2.x_);
        T y = std::min(v1.y_, v2.y_);
        T z = std::min(v1.z_, v2.z_);
        return { x, y, z };
    }

    template <class T>
    Vector3_<T>
    Vector3_<T>::maximum(const Vector3_<T>& v1,
                                         const Vector3_<T>& v2) {
        T x = std::max(v1.x(), v2.x());
        T y = std::max(v1.y(), v2.y());
        T z = std::max(v1.z(), v2.z());
        return { x, y, z };
    }

    template <class T>
    T Vector3_<T>::get(int d) const {
        Assertion(0 <= d && d <= 2, "Dimension must be between 0 and 2");
        if (d == 0) return x_;
        if (d == 1) return y_;
        if (d == 2) return z_;
        return 0.0;
    }

    template <class T>
    std::string Vector3_<T>::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(8);
        ss << "(" << x_ << ", " << y_ << ", " << z_ << ")";
        return std::move(ss.str());
    }

}  // namespace spica

template <class T>
spica::Vector3_<T>
operator+(const spica::Vector3_<T>& v1,
          const spica::Vector3_<T>& v2) {
    spica::Vector3_<T> ret = v1;
    ret += v2;
    return ret;
}

template <class T>
spica::Vector3_<T>
operator-(const spica::Vector3_<T>& v1,
          const spica::Vector3_<T>& v2) {
    spica::Vector3_<T> ret = v1;
    ret -= v2;
    return ret;
}

template <class T>
spica::Vector3_<T>
operator+(const spica::Vector3_<T>& v, T x) {
    spica::Vector3_<T> ret = v;
    ret += x;
    return ret;
}

template <class T>
spica::Vector3_<T>
operator+(T x, const spica::Vector3_<T>& v) {
    return v + x;
}

template <class T>
spica::Vector3_<T>
operator-(const spica::Vector3_<T>& v, T x) {
    return v + (-x);
}

template <class T>
spica::Vector3_<T>
operator-(T x, const spica::Vector3_<T>& v) {
    return x + (-v);
}

template <class T>
spica::Vector3_<T>
operator*(const spica::Vector3_<T>& u,
          const spica::Vector3_<T>& v) {
    spica::Vector3_<T> ret = u;
    ret *= v;
    return ret;
}

template <class T>
spica::Vector3_<T>
operator*(const spica::Vector3_<T>& v, double s) {
    spica::Vector3_<T> ret = v;
    ret *= s;
    return ret;
}

template <class T>
spica::Vector3_<T>
operator*(double s, const spica::Vector3_<T>& v) {
    spica::Vector3_<T> ret = v;
    ret *= s;
    return ret;
}

template <class T>
spica::Vector3_<T>
operator/(const spica::Vector3_<T>& u,
          const spica::Vector3_<T>& v) {
    spica::Vector3_<T> ret = u;
    ret /= v;
    return ret;
}

template <class T>
spica::Vector3_<T>
operator/(const spica::Vector3_<T>& v, double s) {
    spica::Vector3_<T> ret = v;
    ret /= s;
    return ret;
}

template <class T>
std::ostream& operator<<(std::ostream& os,
                         const spica::Vector3_<T>& v) {
    os << v.toString();
    return os;
}

#endif  // _SPICA_VECTOR3D_DETAIL_H_
