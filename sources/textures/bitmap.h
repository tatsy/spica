#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BITMAP_H_
#define _SPICA_BITMAP_H_

#include "core/common.h"
#include "core/core.hpp"
#include "core/texture.h"

namespace spica {
    
class SPICA_EXPORTS BitmapTexture : public Texture<Spectrum> {
public:
    BitmapTexture(const Image& image,
                  const std::shared_ptr<TextureMapping2D>& texmap,
                  ImageWrap wrap);
    
    BitmapTexture(RenderParams &params);
    
    Spectrum evaluate(const SurfaceInteraction& intr) const;
    
private:
    // Private field
    std::unique_ptr<MipMap> mipmap_;
    std::shared_ptr<TextureMapping2D> texmap_;
};

SPICA_EXPORT_PLUGIN(BitmapTexture, "Bitmap texture");

}  // namespace spica

#endif  // _SPICA_BITMAP_H_
