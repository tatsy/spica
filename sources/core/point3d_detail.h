#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_POINT3D_DETAIL_H_
#define _SPICA_POINT3D_DETAIL_H_

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
    Point3_<T>::Point3_(const Point3_<T>& p)
        : x_{ p.x_ }
        , y_{ p.y_ }
        , z_{ p.z_ } {
    }

    template <class T>
    Point3_<T>::~Point3_() {
    }

    template <class T>
    Point3_<T>& Point3_<T>::operator=(const Point3_<T>& p) {
        x_ = p.x_;
        y_ = p.y_;
        z_ = p.z_;
        return *this;
    }

}  // namespace spica

#endif  // _SPICA_POINT3D_DETAIL_H_
