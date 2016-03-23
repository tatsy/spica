#define SPICA_API_EXPORT
#include "imagemap.h"

#include "../core/point2d.h"
#include "../math/vector2d.h"
#include "../image/mipmap.h"

namespace spica {

ImageTexture::ImageTexture(const Image& image,
                           std::unique_ptr<TextureMapping2D> texmap,
                           ImageWrap wrap)
    : mipmap_{ std::make_unique<MipMap>(image, wrap) }
    , texmap_{ std::move(texmap) } {
}

Spectrum ImageTexture::evaluate(const SurfaceInteraction& intr) const {
    Vector2d dstdx, dstdy;
    Point2d st = texmap_->map(intr);
    return mipmap_->lookup(st);
}

}  // namespace spica
