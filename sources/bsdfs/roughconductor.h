#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ROUGH_CONDUCTOR_H_
#define _SPICA_ROUGH_CONDUCTOR_H_

#include "core/core.hpp"
#include "core/common.h"
#include "core/spectrum.h"
#include "core/render.hpp"
#include "core/material.h"

namespace spica {

class SPICA_EXPORTS RoughConductor : public SurfaceMaterial {
public:
    RoughConductor(const std::shared_ptr<Texture<Spectrum>>& eta,
                   const std::shared_ptr<Texture<Spectrum>>& k,
                   const std::shared_ptr<Texture<Spectrum>>& uRoughness,
                   const std::shared_ptr<Texture<Spectrum>>& vRoughness,
                   const std::string &distribution = "beckmann",
                   const std::shared_ptr<Texture<Spectrum>>& bump = nullptr,
                   bool remapRoughness = false);
    RoughConductor(RenderParams &params);
    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    std::shared_ptr<Texture<Spectrum>> eta_;
    std::shared_ptr<Texture<Spectrum>> k_;
    std::shared_ptr<Texture<Spectrum>> uRoughness_, vRoughness_;
    std::string distribution_;
    std::shared_ptr<Texture<Spectrum>> bumpMap_;
    bool remapRoughness_;
};

SPICA_EXPORT_PLUGIN(RoughConductor, "Rough conductor surface");

}  // namespace spica

#endif  // _SPICA_ROUGH_CONDUCTOR_H_
