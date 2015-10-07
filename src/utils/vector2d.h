#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VECTOR2D_H_
#define _SPICA_VECTOR2D_H_

#include <type_traits>

namespace spica {

    template <class T, class Enable = void>
    class Vector2_;

    template <class T>
    using is_arith_t = typename std::enable_if<std::is_arithmetic<T>::value>::type;

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

        T x() const { return _x; }
        T y() const { return _y; }
    };

    using Vector2D = Vector2_<double>;

}  // namespace spica

#include "vector2d_detail.h"

#endif  // _SPICA_VECTOR2D_H_
