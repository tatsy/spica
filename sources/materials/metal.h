#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_METAL_H_
#define _SPICA_METAL_H_

#include "core/core.hpp"
#include "core/common.h"
#include "core/spectrum.h"

#include "render/render.hpp"
#include "render/material.h"

namespace spica {

class SPICA_EXPORTS MetalMaterial : public Material {
public:
    MetalMaterial(const std::shared_ptr<Texture<Spectrum>>& eta,
                  const std::shared_ptr<Texture<Spectrum>>& k,
                  const std::shared_ptr<Texture<double>>& roughness,
                  const std::shared_ptr<Texture<double>>& bump = nullptr,
                  bool remapRoughness = true);
    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    std::shared_ptr<Texture<Spectrum>> eta_;
    std::shared_ptr<Texture<Spectrum>> k_;
    std::shared_ptr<Texture<double>> roughness_;
    std::shared_ptr<Texture<double>> bumpMap_;
    bool remapRoughness_;
};

}  // namespace spica

#endif  // _SPICA_METAL_H_
