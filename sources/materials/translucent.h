#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRANSLUCENT_H_
#define _SPICA_TRANSLUCENT_H_

#include "core/common.h"
#include "core/spectrum.h"

#include "render/render.hpp"
#include "render/material.h"

namespace spica {

class SPICA_EXPORTS TranslucentMaterial : public Material {
public:
    TranslucentMaterial(const std::shared_ptr<Texture<Spectrum>>& Kd,
                        const std::shared_ptr<Texture<Spectrum>>& Ks,
                        const std::shared_ptr<Texture<Spectrum>>& Kr,
                        const std::shared_ptr<Texture<Spectrum>>& Kt,
                        const std::shared_ptr<Texture<double>>& rough = nullptr,
                        const std::shared_ptr<Texture<double>>& bumpMap = nullptr,
                        bool remapRoughness = false);

    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    // Private fields
    std::shared_ptr<Texture<Spectrum>> Kd_, Ks_, Kr_, Kt_;
    std::shared_ptr<Texture<double>> roughness_, bumpMap_;
    bool remapRoughness_;

};  // class TranslucentMaterial

}  // namespace spica

#endif  // _SPICA_TRANSLUCENT_H_
