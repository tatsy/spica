#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BOUNDS2D_H_
#define _SPICA_BOUNDS2D_H_

#include <type_traits>

#include "common.h"
#include "point2d.h"

namespace spica {

template <class T>
class Bounds2_ {
public:
    // Public methods
    Bounds2_();
    Bounds2_(T minx, T miny, T maxx, T maxy);
    Bounds2_(const Bounds2_<T>& b);
    ~Bounds2_();

    Bounds2_<T>& operator=(const Bounds2_<T>& b);
    inline const Point2_<T>& posMin() const { return posMin_; }
    inline const Point2_<T>& posMax() const { return posMax_; }
    inline T width() const { return posMax_.x() - posMin_.x(); }
    inline T height() const { return posMax_.y() - posMin_.y(); }

private:
    // Private fields
    Point2_<T> posMin_, posMax_;

    static_assert(std::is_arithmetic<T>::value,
                  "Template type must be arithmetic!!");

};  // class Bounds2_

using Bounds2i = Bounds2_<int>;
using Bounds2f = Bounds2_<float>;
using Bounds2d = Bounds2_<double>;

}  // namespace spica

#include "bounds2d_detail.h"

#endif  // _SPICA_BOUNDS2D_H_
