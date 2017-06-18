#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_GLASS_H_
#define _SPICA_GLASS_H_

#include <memory>

#include "../core/common.h"
#include "../core/spectrum.h"

#include "material.h"

namespace spica {

class SPICA_EXPORTS GlassMaterial : public Material {
public:
    // Public methods
    GlassMaterial(const std::shared_ptr<Texture<Spectrum>>& Kr,
                  const std::shared_ptr<Texture<Spectrum>>& Kt,
                  const std::shared_ptr<Texture<double>>& uRoughness,
                  const std::shared_ptr<Texture<double>>& vRoughness,
                  const std::shared_ptr<Texture<double>>& index,
                  const std::shared_ptr<Texture<double>>& bumpMap = nullptr,
                  bool remapRoughness = false);
    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    // Private fields
    std::shared_ptr<Texture<Spectrum>> Kr_, Kt_;
    std::shared_ptr<Texture<double>> uRoughness_, vRoughness_;
    std::shared_ptr<Texture<double>> index_;
    std::shared_ptr<Texture<double>> bumpMap_;
    bool remapRoughness_;
};

}  // namespace spica

#endif  // _SPICA_GLASS_H_
