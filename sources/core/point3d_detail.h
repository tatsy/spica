#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_POINT3D_DETAIL_H_
#define _SPICA_POINT3D_DETAIL_H_

#include <iostream>
#include <iomanip>
#include <sstream>

namespace spica {

    template <class T>
    Point3_<T>::Point3_()
        : x_{ 0 }
        , y_{ 0 }
        , z_{ 0 } {
    }

    template <class T>
    Point3_<T>::Point3_(T x, T y, T z)
        : x_{ x }
        , y_{ y }
        , z_{ z } {
    }

    template <class T>
    Point3_<T>::Point3_(const Vector3_<T>& v)
        : x_{ v.x() }
        , y_{ v.y() }
        , z_{ v.z() } {
    }

    template <class T>
    Point3_<T>::Point3_(const Point3_<T>& p)
        : x_{ p.x_ }
        , y_{ p.y_ }
        , z_{ p.z_ } {
    }

    template <class T>
    Point3_<T>::~Point3_() {
    }

    template <class T>
    template <class Fn>
    Point3_<T> Point3_<T>::binary_op(const Point3_<T>& p1, const Point3_<T>& p2, const Fn& f) {
        return { f(p1.x(), p2.x()), f(p1.y(), p2.y()), f(p1.z(), p2.z()) };
    }

    template <class T>
    Point3_<T> Point3_<T>::maximum(const Point3_<T>& p1, const Point3_<T>& p2) {
        static auto max_op = [](T a, T b) -> T { return std::max(a, b); };
        return Point3_<T>::binary_op(p1, p2, max_op);
    }

    template <class T>
    Point3_<T> Point3_<T>::minimum(const Point3_<T>& p1, const Point3_<T>& p2) {
        static auto min_op = [](T a, T b) -> T { return std::min(a, b); };
        return Point3_<T>::binary_op(p1, p2, min_op);        
    }

    template <class T>
    Point3_<T>& Point3_<T>::operator=(const Point3_<T>& p) {
        x_ = p.x_;
        y_ = p.y_;
        z_ = p.z_;
        return *this;
    }

    template <class T>
    Point3_<T>& Point3_<T>::operator+=(const Point3_<T>& p) {
        this->x_ += p.x_;
        this->y_ += p.y_;
        this->z_ += p.z_;
        return *this;
    }

    //template <class T>
    //Point3_<T>& Point3_<T>::operator-=(const Point3_<T>& p) {
    //    this->x_ -= p.x_;
    //    this->y_ -= p.y_;
    //    this->z_ -= p.z_;
    //    return *this;
    //}

    template <class T>
    Point3_<T>& Point3_<T>::operator*=(T s) {
        this->x_ *= s;
        this->y_ *= s;
        this->z_ *= s;
        return *this;
    }

    template <class T>
    Point3_<T>& Point3_<T>::operator/=(T s) {
        Assertion(s != 0.0, "Zero division!!");
        this->x_ /= s;
        this->y_ /= s;
        this->z_ /= s;
        return *this;
    }

    template <class T>
    T Point3_<T>::operator[](int i) const {
        Assertion(i >= 0 && i <= 2, "Index out of bounds!!");
        if (i == 0) return x_;
        if (i == 1) return y_;
        return z_;
    }

    template <class T>
    Point3_<T>::operator Vector3_<T>() const {
        return { x_, y_, z_ };
    }

    template <class T>
    std::string Point3_<T>::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(8);
        ss << "(" << x_ << ", " << y_ << ", " << z_ << ")";
        return std::move(ss.str());
    }

}  // namespace spica

template <class T>
std::ostream& operator<<(std::ostream& os, const spica::Point3_<T>& p) {
    os << p.toString();
    return os;
}

template <class T>
spica::Point3_<T> operator*(const spica::Point3_<T>& p, T s) {
    spica::Point3_<T> ret = p;
    ret *= s;
    return ret;
}

template <class T>
spica::Point3_<T> operator*(T s, const spica::Point3_<T>& p) {
    spica::Point3_<T> ret = p;
    ret *= s;
    return ret;
}

template <class T>
spica::Point3_<T> operator/(const spica::Point3_<T>& p, T s) {
    spica::Point3_<T> ret = p;
    ret *= s;
    return ret;
}

#endif  // _SPICA_POINT3D_DETAIL_H_
