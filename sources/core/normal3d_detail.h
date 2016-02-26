#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_NORMAL3D_DETAIL_H_
#define _SPICA_NORMAL3D_DETAIL_H_

#include <iostream>
#include <iomanip>
#include <sstream>

namespace spica {

    template <class T>
    Normal3_<T>::Normal3_()
        : x_{ 0 }
        , y_{ 0 }
        , z_{ 0 } {
    }

    template <class T>
    Normal3_<T>::Normal3_(T x, T y, T z)
        : x_{ x }
        , y_{ y }
        , z_{ z } {    
    }

    template <class T>
    Normal3_<T>::Normal3_(const Vector3_<T>& v)
        : x_{ v.x() }
        , y_{ v.y() }
        , z_{ v.z() } {
    }

    template <class T>
    Normal3_<T>::Normal3_(const Normal3_<T>& n)
        : x_{ n.x_ }
        , y_{ n.y_ }
        , z_{ n.z_ } {
    }

    template <class T>
    Normal3_<T>::~Normal3_() {
    }

    template <class T>
    Normal3_<T>& Normal3_<T>::operator=(const Normal3_<T>& n) {
        x_ = n.x_;
        y_ = n.y_;
        z_ = n.z_;
        return *this;
    }

    template <class T>
    Normal3_<T> Normal3_<T>::operator-() const {
        return { -x_, -y_, -z_ };
    }

    template <class T>
    Normal3_<T> Normal3_<T>::operator+=(const Normal3_<T>& n) {
        this->x_ += n.x_;
        this->y_ += n.y_;
        this->z_ += n.z_;
        return *this;
    }

    template <class T>
    Normal3_<T> Normal3_<T>::operator-=(const Normal3_<T>& n) {
        this->x_ -= n.x_;
        this->y_ -= n.y_;
        this->z_ -= n.z_;
        return *this;
    }

    template <class T>
    Normal3_<T> Normal3_<T>::operator*=(T s) {
        this->x_ *= s;
        this->y_ *= s;
        this->z_ *= s;
        return *this;
    }

    template <class T>
    Normal3_<T> Normal3_<T>::operator/=(T s) {
        Assertion(s != 0.0, "Zero division!!");
        return this->operator*=(1.0 / s);
    }

    template <class T>
    T Normal3_<T>::operator[](int i) const {
        Assertion(0 <= i && i <= 2, "Index out of bounds!!");
        if (i == 0) return x_;
        if (i == 1) return y_;
        return z_;
    }

    template <class T>
    Normal3_<T>::operator Vector3_<T>() const {
        return { x_, y_, z_ };
    }

    template <class T>
    std::string Normal3_<T>::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(8);
        ss << "(" << x_ << ", " << y_ << ", " << z_ << ")";
        return std::move(ss.str());
    }

    template <class T>
    double Normal3_<T>::norm() const {
        return sqrt(squaredNorm());
    }

    template <class T>
    double Normal3_<T>::squaredNorm() const {
        return x_ * x_ + y_ * y_ + z_ * z_;
    }

    template <class T>
    Normal3_<T> Normal3_<T>::normalized() const {
        return (*this) / this->norm();
    }

}  // namespace spica

template <class T>
std::ostream& operator<<(std::ostream& os, const spica::Normal3_<T>& n) {
    os << n.toString();
    return os;
}

template <class T>
spica::Normal3_<T> operator+(const spica::Normal3_<T>& n1, const spica::Normal3_<T>& n2) {
    spica::Normal3_<T> ret = n1;
    ret += n2;
    return ret;
}

template <class T>
spica::Normal3_<T> operator-(const spica::Normal3_<T>& n1, const spica::Normal3_<T>& n2) {
    spica::Normal3_<T> ret = n1;
    ret -= n2;
    return ret;
}

template <class T>
spica::Normal3_<T> operator*(const spica::Normal3_<T>& n, T s) {
    spica::Normal3_<T> ret = n;
    ret *= s;
    return ret;
}

template <class T>
spica::Normal3_<T> operator*(T s, const spica::Normal3_<T>& n) {
    spica::Normal3_<T> ret = n;
    ret *= s;
    return ret;
}

template <class T>
spica::Normal3_<T> operator/(const spica::Normal3_<T>& n, T s) {
    spica::Normal3_<T> ret = n;
    ret /= s;
    return ret;
}


#endif  // _SPICA_NORMAL3D_DETAIL_H_
