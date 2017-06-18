#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_KDSUBSURFACE_H_
#define _SPICA_KDSUBSURFACE_H_

#include <memory>

#include "core/core.hpp"
#include "core/common.h"
#include "core/spectrum.h"
#include "core/interpolation.h"

#include "render/render.hpp"
#include "render/material.h"

namespace spica {

class SPICA_EXPORTS KdSubsurfaceMaterial : public Material {
public:
    // Public methods
    KdSubsurfaceMaterial(double scale,
                         const std::shared_ptr<Texture<Spectrum>>& Kd,
                         const std::shared_ptr<Texture<Spectrum>>& Kr,
                         const std::shared_ptr<Texture<Spectrum>>& Kt,
                         const std::shared_ptr<Texture<Spectrum>>& mfp,
                         double g, double eta,
                         const std::shared_ptr<Texture<double>>& uRoughness = nullptr,
                         const std::shared_ptr<Texture<double>>& vRoughness = nullptr,
                         const std::shared_ptr<Texture<double>>& bumpMap = nullptr,
                         bool remapRoughness = false);

    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    // Private fields
    double scale_;
    std::shared_ptr<Texture<Spectrum>> Kd_, Kr_, Kt_, mfp_;
    std::shared_ptr<Texture<double>> uRoughness_, vRoughness_, bumpMap_;
    double eta_;
    bool remapRoughness_;
    CatmullRom2D table_;
};

}  // namespace spica

#endif  // _SPICA_KDSUBSURFACE_H_
