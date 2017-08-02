#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PLASTIC_H_
#define _SPICA_PLASTIC_H_

#include "core/common.h"
#include "core/spectrum.h"

#include "core/render.hpp"
#include "core/material.h"

namespace spica {

class SPICA_EXPORTS PlasticMaterial : public Material {
public:
    PlasticMaterial(const std::shared_ptr<Texture<Spectrum>>& Kd,
                    const std::shared_ptr<Texture<Spectrum>>& Ks,
                    const std::shared_ptr<Texture<double>>& roughness = nullptr,
                    const std::shared_ptr<Texture<double>>& bumpMap = nullptr,
                    bool remapRoughness = false);

    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    std::shared_ptr<Texture<Spectrum>> Kd_, Ks_;
    std::shared_ptr<Texture<double>> roughness_, bumpMap_;
    bool remapRoughness_;
};

}  // namespace spica

#endif  // _SPICA_PLASTIC_H_
