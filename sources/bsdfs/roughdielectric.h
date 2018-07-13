#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ROUGH_DIELECTRIC_H_
#define _SPICA_ROUGH_DIELECTRIC_H_

#include <memory>

#include "core/common.h"
#include "core/spectrum.h"
#include "core/render.hpp"
#include "core/material.h"

namespace spica {

class SPICA_EXPORTS RoughDielectric : public SurfaceMaterial {
public:
    // Public methods
    RoughDielectric(const std::shared_ptr<Texture<Spectrum>>& Kr,
                    const std::shared_ptr<Texture<Spectrum>>& Kt,
                    const std::shared_ptr<Texture<double>>& uRoughness,
                    const std::shared_ptr<Texture<double>>& vRoughness,
                    const std::shared_ptr<Texture<double>>& index,
                    const std::string &distribution = "beckmann",
                    const std::shared_ptr<Texture<double>>& bumpMap = nullptr,
                    bool remapRoughness = false);

    explicit RoughDielectric(RenderParams &params);

    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    // Private fields
    std::shared_ptr<Texture<Spectrum>> Kr_, Kt_;
    std::shared_ptr<Texture<double>> uRoughness_, vRoughness_;
    std::shared_ptr<Texture<double>> index_;
    std::string distribution_;
    std::shared_ptr<Texture<double>> bumpMap_;
    bool remapRoughness_;
};

SPICA_EXPORT_PLUGIN(RoughDielectric, "Rough dielectric surface");

}  // namespace spica

#endif  // _SPICA_ROUGH_DIELECTRIC_H_
