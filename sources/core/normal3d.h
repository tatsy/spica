#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_NORMAL3D_H_
#define _SPICA_NORMAL3D_H_

#include <type_traits>

#include "../math/vect_math.h"

namespace spica {

    template <class T>
    class Normal3_ {
    public:
        Normal3_();
        Normal3_(T x, T y, T z);
        Normal3_(const Vector3_<T>& v);
        Normal3_(const Normal3_<T>& n);
        ~Normal3_();

        Normal3_<T>& operator=(const Normal3_<T>& n);

        Normal3_<T> operator-() const;

        explicit operator Vector3_<T>() const;

        inline T x() const { return x_; }
        inline T y() const { return y_; }
        inline T z() const { return z_; }
        
    private:
        T x_, y_, z_;

        static_assert(std::is_arithmetic<T>::value, "Template type must be arithmetic!!");
    };

    using Normal3D = Normal3_<double>;
    using Normal   = Normal3D;

}  // namespace spica

#include "normal3d_detail.h"

#endif  // _SPICA_NORMAL3D_H_
