#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BSPHERE_H_
#define _SPICA_BSPHERE_H_

#include "point3d.h"

namespace spica {

template <class T>
class BSphere_ {
public:
    BSphere_();
    BSphere_(const Point3d &center, double radius);
    BSphere_(const BSphere_ &sph);

    BSphere_ & operator=(const BSphere_ &sph);

    inline Point3_<T> center() const { return center_; }
    inline T radius() const { return radius_; }

private:
    Point3_<T> center_;
    T radius_;

    static_assert(std::is_floating_point<T>::value,
                  "Template type must be floating point type!");
};

using BSphere = BSphere_<double>;

}  // namespace spica

#include "bsphere_detail.h"

#endif  // _SPICA_BSPHERE_H_