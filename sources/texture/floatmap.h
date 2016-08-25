#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FLOATMAP_H_
#define _SPICA_FLOATMAP_H_

#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "texture.h"

namespace spica {

class SPICA_EXPORTS FloatTexture : public Texture<double> {
public:
    FloatTexture(const Image& image,
                 const std::shared_ptr<TextureMapping2D>& texmap,
                 ImageWrap wrap);

    double evaluate(const SurfaceInteraction& intr) const;

private:
    // Private field
    std::unique_ptr<MipMap> mipmap_;
    std::shared_ptr<TextureMapping2D> texmap_;
};

}  // namespace spica

#endif  // _SPICA_FLOATMAP_H_
