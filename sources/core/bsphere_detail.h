#ifndef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BSPHERE_DETAIL_H_
#define _SPICA_BSPHERE_DETAIL_H_

namespace spica {

template <class T>
BSphere_<T>::BSphere_()
    : center_{} 
    , radius_{} {
}

template <class T>
BSphere_<T>::BSphere_(const Point3d &center, double radius)
    : center_{center}
    , radius_{radius} {
}

template <class T>
BSphere_<T>::BSphere_(const BSphere_<T> &sph)
    : BSphere_{} {
    this->operator=(sph);
}

template <class T>
BSphere_<T> & BSphere_<T>::operator=(const BSphere_<T> &sph) {
    this->center_ = sph.center_;
    this->radius_ = sph.radius_;
    return *this;
}

}  // namespace spica

#endif  // _SPICA_BSPHERE_DETAIL_H_
