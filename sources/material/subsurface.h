#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SUBSURFACE_H_
#define _SPICA_SUBSURFACE_H_

#include "../core/common.h"
#include "../core/spectrum.h"
#include "../core/interpolation.h"

#include "material.h"

namespace spica {

class SPICA_EXPORTS SubsurfaceMaterial : public Material {
public:
    // Public methods
    SubsurfaceMaterial(double scale,
                       const std::shared_ptr<Texture<Spectrum>>& Kr,
                       const std::shared_ptr<Texture<Spectrum>>& Kt,
                       const std::shared_ptr<Texture<Spectrum>>& sigmaAbsorb,
                       const std::shared_ptr<Texture<Spectrum>>& sigmaScatter,
                       double g, double eta,
                       const std::shared_ptr<Texture<double>>& uRoughness = nullptr,
                       const std::shared_ptr<Texture<double>>& vRoughness = nullptr,
                       const std::shared_ptr<Texture<double>>& bumpMap = nullptr,
                       bool remapRoughness = true);

    void setScatterFuncs(SurfaceInteraction* isect,
                         MemoryArena& arena) const override;

private:
    // Private fields
    double scale_;
    std::shared_ptr<Texture<Spectrum>> Kr_, Kt_, sigmaAbsorb_, sigmaScatter_;
    std::shared_ptr<Texture<double>> uRoughness_, vRoughness_, bumpMap_;
    double eta_;
    bool remapRoughness_;
    CatmullRom2D table_;

};  // class SubsurfaceMaterial

}  // namespace spica

#endif  // _SPICA_SUBSURFACE_H_
