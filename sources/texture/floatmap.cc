#define SPICA_API_EXPORT
#include "floatmap.h"

#include "../core/point2d.h"
#include "../math/vector2d.h"
#include "../image/mipmap.h"

namespace spica {

FloatTexture::FloatTexture(const Image& image,
                           const std::shared_ptr<TextureMapping2D>& texmap,
                           ImageWrap wrap)
    : mipmap_{ std::make_unique<MipMap>(image, wrap) }
    , texmap_{ texmap } {
}

double FloatTexture::evaluate(const SurfaceInteraction& intr) const {
   Vector2d dstdx, dstdy;
    Point2d st = texmap_->map(intr);
    return mipmap_->lookup(st).luminance();
}

}  // namespace spica
