#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECTOR2D_DETAIL_H_
#define _SPICA_VECTOR2D_DETAIL_H_

namespace spica {

    template <class T>
    Vector2_<T, is_arith_t<T> >::Vector2_()
        : _x(0)
        , _y(0) {
    }

    template <class T>
    Vector2_<T, typename is_arith_t<T> >::Vector2_(T x, T y)
        : _x(x)
        , _y(y) {
    }

    template <class T>
    Vector2_<T, is_arith_t<T> >::Vector2_(const Vector2_<T, is_arith_t<T> >& v)
        : _x(v._x)
        , _y(v._y) {
    }

    template <class T>
    Vector2_<T, is_arith_t<T> >::~Vector2_() {
    }

    template <class T>
    Vector2_<T, is_arith_t<T> >&
    Vector2_<T, is_arith_t<T> >::operator=(const Vector2_<T, is_arith_t<T> >& v) {
        this->_x = v._x;
        this->_y = v._y;
        return *this;
    }

}  // namespace spica

#endif  // _SPICA_VECTOR2D_DETAIL_H_
