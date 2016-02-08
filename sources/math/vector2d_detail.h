#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECTOR2D_DETAIL_H_
#define _SPICA_VECTOR2D_DETAIL_H_

namespace spica {

    template <class T>
    Vector2_<T, is_arith_t<T> >::Vector2_()
        : _x{0}
        , _y{0} {
    }

    template <class T>
    Vector2_<T, is_arith_t<T> >::Vector2_(T x, T y)
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

    template <class T>
    Vector2_<T, is_arith_t<T> >&
    Vector2_<T, is_arith_t<T> >::operator+=(const Vector2_<T, is_arith_t<T> >& v) {
        this->_x += v._x;
        this->_y += v._y;
        return *this;
    }

    template <class T>
    Vector2_<T, is_arith_t<T> >&
    Vector2_<T, is_arith_t<T> >::operator-=(const Vector2_<T, is_arith_t<T> >& v) {
        this->_x -= v._x;
        this->_y -= v._y;
        return *this;
    }

    template <class T>
    Vector2_<T, is_arith_t<T> >&
    Vector2_<T, is_arith_t<T> >::operator*=(double s) {
        this->_x *= s;
        this->_y *= s;
        return *this;
    }

}  // namespace spica

template <class T>
spica::Vector2_<T, is_arith_t<T> > operator+(const spica::Vector2_<T, is_arith_t<T> >& v1,
                                             const spica::Vector2_<T, is_arith_t<T> >& v2) {
    spica::Vector2_<T, is_arith_t<T> > ret = v1;
    ret += v2;
    return ret;
}

template <class T>
spica::Vector2_<T, is_arith_t<T> > operator-(const spica::Vector2_<T, is_arith_t<T> >& v1,
                                             const spica::Vector2_<T, is_arith_t<T> >& v2) {
    spica::Vector2_<T, is_arith_t<T> > ret = v1;
    ret -= v2;
    return ret;
}

template <class T>
spica::Vector2_<T, is_arith_t<T> > operator*(const spica::Vector2_<T, is_arith_t<T> >& v, double s) {
    spica::Vector2_<T, is_arith_t<T> > ret = v;
    ret *= s;
    return ret;
}

template <class T>
spica::Vector2_<T, is_arith_t<T> > operator*(double s, const spica::Vector2_<T, is_arith_t<T> >& v) {
    spica::Vector2_<T, is_arith_t<T> > ret = v;
    ret *= s;
    return ret;
}

#endif  // _SPICA_VECTOR2D_DETAIL_H_
