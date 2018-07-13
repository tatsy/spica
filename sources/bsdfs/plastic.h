#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PLASTIC_H_
#define _SPICA_PLASTIC_H_

#include "core/common.h"
#include "core/cobject.h"
#include "core/spectrum.h"
#include "core/render.hpp"
#include "core/material.h"
#include "core/constant.h"

namespace spica {

class SPICA_EXPORTS Plastic : public SurfaceMaterial {
public:
    Plastic(const std::shared_ptr<Texture<Spectrum>> &Kd,
            const std::shared_ptr<Texture<Spectrum>> &Ks,
            const std::shared_ptr<Texture<double>> &eta = std::make_shared<ConstantTexture<double>>(1.5),
            const std::shared_ptr<Texture<double>> &bumpMap = nullptr);

    explicit Plastic(RenderParams &params);

    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    std::shared_ptr<Texture<Spectrum>> Kd_, Ks_;
    std::shared_ptr<Texture<double>> eta_, bumpMap_;
};

SPICA_EXPORT_PLUGIN(Plastic, "Plastic-like surface");

}  // namespace spica

#endif  // _SPICA_PLASTIC_H_
