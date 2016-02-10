#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_POINT3D_H_
#define _SPICA_POINT3D_H_

#include <string>
#include <type_traits>

#include "common.h"
#include "../math/vector3d.h"

namespace spica {

    template <class T>
    class Point3_ {
    public:
        Point3_();
        Point3_(T x, T y, T z);
        Point3_(const Point3_<T>& p);
        explicit Point3_(const Vector3_<T>& v);
        ~Point3_();

        template <class Fn>
        static Point3_<T> binary_op(const Point3_<T>& p1, const Point3_<T>& p2, const Fn& f);

        static Point3_<T> maximum(const Point3_<T>& p1, const Point3_<T>& p2);
        static Point3_<T> minimum(const Point3_<T>& p1, const Point3_<T>& p2);

        Point3_<T>& operator=(const Point3_<T>& p);
        Point3_<T>& operator+=(const Point3_<T>& p);
        Point3_<T>& operator*=(T s);
        Point3_<T>& operator/=(T s);
        T operator[](int i) const;
        operator Vector3_<T>() const;

        inline T x() const { return x_; }
        inline T y() const { return y_; }
        inline T z() const { return z_; }

        std::string toString() const;

    private:
        T x_, y_, z_;

        static_assert(std::is_arithmetic<T>::value, "Template type must be arithmetic!!");
    };

    using Point3D = Point3_<double>;
    using Point   = Point3D;

}  // namespace spica

template <class T>
std::ostream& operator<<(std::ostream& os, const spica::Point3_<T>& p);

template <class T>
spica::Point3_<T> operator+(const spica::Point3_<T>& p1, const spica::Point3_<T>& p2);

template <class T>
spica::Point3_<T> operator+(const spica::Point3_<T>& p, const spica::Vector3_<T>& v);

template <class T>
spica::Point3_<T> operator-(const spica::Point3_<T>& p, const spica::Vector3_<T>& v);

template <class T>
spica::Vector3_<T> operator-(const spica::Point3_<T>& p1, const spica::Point3_<T>& p2);

template <class T>
spica::Point3_<T> operator*(const spica::Point3_<T>& p, T s);

template <class T>
spica::Point3_<T> operator*(T s, const spica::Point3_<T>& p);

template <class T>
spica::Point3_<T> operator/(const spica::Point3_<T>& p, T s);

#include "point3d_detail.h"

#endif  // _SPICA_POINT3D_H_
