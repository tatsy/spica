#define SPICA_API_EXPORT
#include "checkerboard.h"

#include "core/interaction.h"

namespace spica {

Checkerboard::Checkerboard(const Spectrum &color0, const Spectrum &color1,
    double uOffset, double vOffset, double uScale, double vScale)
    : color0_{color0}
    , color1_{color1}
    , uOffset_{uOffset}
    , vOffset_{vOffset}
    , uScale_{uScale}
    , vScale_{vScale} {
}

Checkerboard::Checkerboard(RenderParams &params)
    : Checkerboard{params.getSpectrum("color0"),
                   params.getSpectrum("color1"),
                   params.getDouble("uoffset"),
                   params.getDouble("voffset"),
                   params.getDouble("uscale"),
                   params.getDouble("vscale")} {
}

Spectrum Checkerboard::evaluate(const SurfaceInteraction &intr) const {
    Point2d uv = intr.uv();
    const int iu = static_cast<int>((uv.x() * uScale_ + uOffset_) * 2.0);
    const int iv = static_cast<int>((uv.y() * vScale_ + vOffset_) * 2.0);
    if ((iu + iv) % 2 != 0) {
        return color0_;
    } else {
        return color1_;
    }
}

}  // namespace spica
