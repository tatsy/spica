#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECTOR2D_H_
#define _SPICA_VECTOR2D_H_

#include <type_traits>

template <class T>
using is_arith_t = typename std::enable_if<std::is_arithmetic<T>::value>::type;

namespace spica {

    template <class T, class Enable = void>
    class Vector2_;

    /** Two-dimensional vector.
     *  @ingroup math_module
     */
    template <class T>
    class Vector2_<T, is_arith_t<T> > {
    private:
        T _x, _y;

    public:
        Vector2_();
        Vector2_(T x, T y);
        Vector2_(const Vector2_& v);
        
        ~Vector2_();
    
        Vector2_& operator=(const Vector2_& v);
        Vector2_& operator+=(const Vector2_& v);
        Vector2_& operator-=(const Vector2_& v);
        Vector2_& operator*=(double s);

        T x() const { return _x; }
        T y() const { return _y; }
    };

    using Vector2D = Vector2_<double>;

}  // namespace spica

template <class T>
spica::Vector2_<T, is_arith_t<T> > operator+(const spica::Vector2_<T, is_arith_t<T> >& v1,
                                             const spica::Vector2_<T, is_arith_t<T> >& v2);

template <class T>
spica::Vector2_<T, is_arith_t<T> > operator-(const spica::Vector2_<T, is_arith_t<T> >& v1,
                                             const spica::Vector2_<T, is_arith_t<T> >& v2);

template <class T>
spica::Vector2_<T, is_arith_t<T> > operator*(const spica::Vector2_<T, is_arith_t<T> >& v, double s);

template <class T>
spica::Vector2_<T, is_arith_t<T> > operator*(double s, const spica::Vector2_<T, is_arith_t<T> >& v);

#include "vector2d_detail.h"

#endif  // _SPICA_VECTOR2D_H_
