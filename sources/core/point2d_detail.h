#include "point2d.h"

#include "common.h"

namespace spica {

    template <class T>
    Point2_<T>::Point2_()
        : x_{ 0 }
        , y_{ 0 } {
    }

    template <class T>
    Point2_<T>::Point2_(T x, T y)
        : x_{ x }
        , y_{ y } {
    }

    template <class T>
    Point2_<T>::Point2_(const Point2_<T>& p)
        : x_{ p.x_ }
        , y_{ p.y_ } {
    }

    template <class T>
    Point2_<T>& Point2_<T>::operator=(const Point2_<T>& p) {
        this->x_ = p.x_;
        this->y_ = p.y_;
        return *this;
    }

    template <class T>
    Point2_<T> Point2_<T>::operator-() const {
        return { -x_, -y_ };
    }

    template <class T>
    Point2_<T>& Point2_<T>::operator+=(const Point2_<T>& p) {
        this->x_ += p.x_;
        this->y_ += p.y_;
        return *this;
    }

    template <class T>
    Point2_<T>& Point2_<T>::operator-=(const Point2_<T>& p) {
        this->x_ -= p.x_;
        this->y_ -= p.y_;
        return *this;
    }

    template <class T>
    T Point2_<T>::operator[](int i) const {
        Assertion(i >= 0 && i <= 1, "Index out of bounds!!");
        if (i == 0) {
            return x_;
        } else {
            return y_;
        }
    }

    template <class T>
    std::string Point2_<T>::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(8);
        ss << "(" << x_ << ", " << y_ << ")";
        return std::move(ss.str());        
    }

}  // namespace spica

template <class T>
std::ostream& operator<<(std::ostream& os, const spica::Point2_<T>& p) {
    os << p.toString();
    return os;
}

template <class T>
spica::Point2_<T> operator+(const spica::Point2_<T>& p1, const spica::Point2_<T>& p2) {
    spica::Point2_<T> ret = p1;
    ret += p2;
    return ret;
}

template <class T>
spica::Point2_<T> operator-(const spica::Point2_<T>& p1, const spica::Point2_<T>& p2) {
    spica::Point2_<T> ret = p1;
    ret -= p2;
    return ret;
}
