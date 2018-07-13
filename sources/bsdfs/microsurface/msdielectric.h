#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MS_DIELECTRIC_H_
#define _SPICA_MS_DIELECTRIC_H_

#include "core/common.h"
#include "core/material.h"

namespace spica {

/**
 * Microsurface dielectric material
 * Heitz et al. 2016, "Multiple-Scattering Microfacet BSDFs with the Smith Model"
 */
class SPICA_EXPORTS MicrosurfaceDielectric : public SurfaceMaterial {
public:
    // Public methods
    MicrosurfaceDielectric(const std::shared_ptr<Texture<Spectrum>> &Kr,
                           const std::shared_ptr<Texture<Spectrum>> &Kt,
                           const std::shared_ptr<Texture<double>> &uRoughness,
                           const std::shared_ptr<Texture<double>> &vRoughness,
                           const std::shared_ptr<Texture<double>> &index,
                           const std::string &slopeDistribution,
                           const std::string &heightDistribution,
                           int scatteringOrder = 4,
                           const std::shared_ptr<Texture<double>> &bumpMap = nullptr,
    bool remapRoughness = false);

    explicit MicrosurfaceDielectric(RenderParams &params);

    void setScatterFuncs(SurfaceInteraction *isect,
                         MemoryArena &arena) const override;

private:
    // Private fields
    std::shared_ptr<Texture<Spectrum>> Kr_, Kt_;
    std::shared_ptr<Texture<double>> uRoughness_, vRoughness_;
    std::shared_ptr<Texture<double>> index_;
    std::string slopeDistribution_, heightDistribution_;
    int scatteringOrder_;
    std::shared_ptr<Texture<double>> bumpMap_;
    bool remapRoughness_;
};

SPICA_EXPORT_PLUGIN(MicrosurfaceDielectric, "Microsurface dielectric");

}  // namespace spica

#endif  // _SPICA_MS_SMITH_H_
