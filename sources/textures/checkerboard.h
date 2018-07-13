#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_CHECKERBOARD_H_
#define _SPICA_CHECKERBOARD_H_

#include "core/common.h"
#include "core/texture.h"
#include "core/renderparams.h"

namespace spica {

class SPICA_EXPORTS Checkerboard : public Texture<Spectrum> {
public:
    explicit Checkerboard(const Spectrum &color0, const Spectrum &color1, 
                          double uOffset = 0.0, double vOffset = 0.0,
                          double uScale = 1.0, double vScale = 1.0);
    explicit Checkerboard(RenderParams &params);

    Spectrum evaluate(const SurfaceInteraction &intr) const override;

private:
    Spectrum color0_, color1_;
    double uOffset_, vOffset_, uScale_, vScale_;
};

SPICA_EXPORT_PLUGIN(Checkerboard, "Checkerboard texture");

}  // namespace spica

#endif  // _SPICA_CHECKERBOARD_H_

