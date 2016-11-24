#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BOUNDS2D_DETAIL_H_
#define _SPICA_BOUNDS2D_DETAIL_H_

namespace spica {

template <class T>
Bounds2_<T>::Bounds2_()
    : posMin_{}
    , posMax_{} {
}

template <class T>
Bounds2_<T>::Bounds2_(T minx, T miny, T maxx, T maxy)
    : posMin_{ minx, miny }
    , posMax_{ maxx, maxy } {
}

template <class T>
Bounds2_<T>::Bounds2_(const Bounds2_<T>& b)
    : posMin_{ b.posMin_ }
    , posMax_{ b.posMax_ } {
}

template <class T>
Bounds2_<T>::~Bounds2_() {
}

template <class T>
Bounds2_<T>& Bounds2_<T>::operator=(const Bounds2_<T>& b) {
    this->posMin_ = b.posMin_;
    this->posMax_ = b.posMax_;
    return *this;
}

}  // namespace spica

#endif  // _SPICA_BOUNDS2D_DETAIL_H_
