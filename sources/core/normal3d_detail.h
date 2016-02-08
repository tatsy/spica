#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_NORMAL3D_DETAIL_H_
#define _SPICA_NORMAL3D_DETAIL_H_

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
    Normal3_<T>::operator Vector3_<T>() const {
        return { x_, y_, z_ };
    }

    template <class T>
    Normal3_<T> Normal3_<T>::operator-() const {
        return { -x_, -y_, -z_ };
    }

}  // namespace spica

#endif  // _SPICA_NORMAL3D_DETAIL_H_
