#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_POINT2D_H_
#define _SPICA_POINT2D_H_

#include <string>

namespace spica {

    template <class T>
    class Point2_ {
    public:
        Point2_();
        Point2_(T x, T y);
        Point2_(const Point2_<T>& p);

        Point2_<T>& operator=(const Point2_<T>& p);
        Point2_<T>  operator-() const;
        Point2_<T>& operator+=(const Point2_<T>& p);
        Point2_<T>& operator-=(const Point2_<T>& p);
        T operator[](int i) const;

        inline T x() const { return x_; }
        inline T y() const { return y_; }

        std::string toString() const;

    private:
        T x_, y_;
    };

    using Point2i = Point2_<int>;
    using Point2D = Point2_<double>;

}  // namespace spica

template <class T>
std::ostream& operator<<(std::ostream& os, const spica::Point2_<T>& p);

template <class T>
spica::Point2_<T> operator+(const spica::Point2_<T>& p1, const spica::Point2_<T>& p2);

template <class T>
spica::Point2_<T> operator-(const spica::Point2_<T>& p1, const spica::Point2_<T>& p2);

#endif  // _SPICA_POINT2D_H_
