#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_NORMAL3D_H_
#define _SPICA_NORMAL3D_H_

#include <string>
#include <type_traits>

#include "../math/vect_math.h"

namespace spica {

    template <class T>
    class Normal3_ {
    public:
        Normal3_();
        Normal3_(T x, T y, T z);
        explicit Normal3_(const Vector3_<T>& v);
        Normal3_(const Normal3_<T>& n);
        ~Normal3_();

        Normal3_<T>& operator=(const Normal3_<T>& n);
        bool operator==(const Normal3_<T>& n) const;
        bool operator!=(const Normal3_<T>& n) const;
        Normal3_<T> operator-() const;
        Normal3_<T> operator+=(const Normal3_<T>& n);
        Normal3_<T> operator-=(const Normal3_<T>& n);
        Normal3_<T> operator*=(T s);
        Normal3_<T> operator/=(T s);
        T operator[](int i) const;
        explicit operator Vector3_<T>() const;

        inline T x() const { return x_; }
        inline T y() const { return y_; }
        inline T z() const { return z_; }

        double norm() const;
        double squaredNorm() const;
        Normal3_<T> normalized() const;

        std::string toString() const;

        using type = T;
        
    private:
        T x_, y_, z_;

        static_assert(std::is_arithmetic<T>::value, "Template type must be arithmetic!!");
    };

    using Normal3D = Normal3_<double>;
    using Normal   = Normal3D;

}  // namespace spica

template <class T>
std::ostream& operator<<(std::ostream& os, const spica::Normal3_<T>& n);

template <class T>
spica::Normal3_<T> operator+(const spica::Normal3_<T>& n1, const spica::Normal3_<T>& n2);

template <class T>
spica::Normal3_<T> operator-(const spica::Normal3_<T>& n1, const spica::Normal3_<T>& n2);

template <class T>
spica::Normal3_<T> operator*(const spica::Normal3_<T>& n, T s);

template <class T>
spica::Normal3_<T> operator*(T s, const spica::Normal3_<T>& n);

template <class T>
spica::Normal3_<T> operator/(const spica::Normal3_<T>& n, T s);


#include "normal3d_detail.h"

#endif  // _SPICA_NORMAL3D_H_
