#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ROUGH_PLASTIC_H_
#define _SPICA_ROUGH_PLASTIC_H_

#include "core/common.h"
#include "core/spectrum.h"
#include "core/render.hpp"
#include "core/material.h"
#include "core/constant.h"

namespace spica {

class SPICA_EXPORTS RoughPlastic : public SurfaceMaterial {
public:
    RoughPlastic(const std::shared_ptr<Texture<Spectrum>> &Kd,
                 const std::shared_ptr<Texture<Spectrum>> &Ks,
                 const std::shared_ptr<Texture<Spectrum>> &roughness,
                 const std::shared_ptr<Texture<Spectrum>> &index,
                 const std::string &distribution = "beckmann",
                 const std::shared_ptr<Texture<Spectrum>> &bumpMap = nullptr,
                 bool remapRoughness = false);

    RoughPlastic(RenderParams &params);

    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    std::shared_ptr<Texture<Spectrum>> Kd_, Ks_;
    std::shared_ptr<Texture<Spectrum>> roughness_, index_, bumpMap_;
    std::string distribution_;
    bool remapRoughness_;
};

SPICA_EXPORT_PLUGIN(RoughPlastic, "Rough plastic-like surface");

}  // namespace spica

#endif  // _SPICA_ROUGH_PLASTIC_H_
