#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECTOR2D_H_
#define _SPICA_VECTOR2D_H_

#include <type_traits>

namespace spica {

/** 
 *  Two-dimensional vector.
 *  @ingroup math_module
 */
template <class T>
class Vector2_ {
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

private:
    T _x, _y;

    static_assert(std::is_arithmetic<T>::value,
                  "Template type must be arithmetic!!");
};

using Vector2i = Vector2_<int>;
using Vector2f = Vector2_<float>;
using Vector2d = Vector2_<double>;

}  // namespace spica

template <class T>
spica::Vector2_<T> operator+(const spica::Vector2_<T>& v1,
                             const spica::Vector2_<T>& v2);

template <class T>
spica::Vector2_<T> operator-(const spica::Vector2_<T>& v1,
                             const spica::Vector2_<T>& v2);

template <class T>
spica::Vector2_<T> operator*(const spica::Vector2_<T>& v, double s);

template <class T>
spica::Vector2_<T> operator*(double s, const spica::Vector2_<T>& v);

#include "vector2d_detail.h"

#endif  // _SPICA_VECTOR2D_H_
