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

        Point3_<T>& operator=(const Point3_<T>& p);

        inline T x() const { return x_; }
        inline T y() const { return y_; }
        inline T z() const { return z_; }

    private:
        T x_, y_, z_;

        static_assert(std::is_arithmetic<T>(), "Template type must be arithmetic!!");
    
    };

}  // namespace spica

#include "point3d_detail.h"

#endif  // _SPICA_POINT3D_H_
