#define SPICA_API_EXPORT
#include "bitmap.h"

#include "core/mipmap.h"

namespace spica {

BitmapTexture::BitmapTexture(const Image& image,
                             const std::shared_ptr<TextureMapping2D>& texmap,
                             ImageWrap wrap)
: mipmap_{ std::make_unique<MipMap>(image, wrap) }
, texmap_{ texmap } {
}

BitmapTexture::BitmapTexture(RenderParams &params)
    : BitmapTexture{Image::fromFile(params.getString("filename", true)),
                    std::make_shared<UVMapping2D>(),
                    ImageWrap::Repeat} {
}
    

Spectrum BitmapTexture::evaluate(const SurfaceInteraction& intr) const {
    Vector2d dstdx, dstdy;
    Point2d st = texmap_->map(intr);
    return mipmap_->lookup(st);
}
    
}  // namespace spica
