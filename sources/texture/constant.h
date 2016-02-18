#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TEXTURE_CONSTANT_H_
#define _SPICA_TEXTURE_CONSTANT_H_

#include "texture.h"

namespace spica {

template <class T>
class ConstantTexture : public Texture<T> {
public:
    ConstantTexture(const T& value) : value_{ value } {}
    T evaluate(const SurfaceInteraction& isect) const { return value_; }

private:
    T value_;
};  // class ConstantTexture

}  // namespace spica

#endif  // _SPICA_TEXTURE_CONSTANT_H_
