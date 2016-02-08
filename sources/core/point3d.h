#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_POINT3D_H_
#define _SPICA_POINT3D_H_

#include <type_traits>

namespace spica {

    template <class T>
    class Point3_ {
    public:
        Point3_();
        Point3_(T x, T y, T z);
        Point3_(const Point3_<T>& p);
        ~Point3_();

        template <class Fn>
        static Point3_<T> binary_op(const Point3_<T>& p1, const Point3_<T>& p2, const Fn& f);

        static Point3_<T> maximum(const Point3_<T>& p1, const Point3_<T>& p2);
        static Point3_<T> minimum(const Point3_<T>& p1, const Point3_<T>& p2);

        Point3_<T>& operator=(const Point3_<T>& p);
        T operator[](int i) const;

        inline T x() const { return x_; }
        inline T y() const { return y_; }
        inline T z() const { return z_; }

    private:
        T x_, y_, z_;

        static_assert(std::is_arithmetic<T>::value, "Template type must be arithmetic!!");
    };

    using Point3D = Point3_<double>;
    using Point   = Point3D;

}  // namespace spica

#include "point3d_detail.h"

#endif  // _SPICA_POINT3D_H_
