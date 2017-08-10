#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MS_CONDUCTOR_H_
#define _SPICA_MS_CONDUCTOR_H_

#include "core/common.h"
#include "core/material.h"

namespace spica {

/**
 * BSDF for multiple scattering microsurface reflection
 * Heitz et al. 2016, "Multiple-Scattering Microfacet BSDFs with the Smith Model"
 */
class SPICA_EXPORTS MicrosurfaceConductor : public SurfaceMaterial {
public:
    // Public methods
    MicrosurfaceConductor(const std::shared_ptr<Texture<Spectrum>> &eta,
                          const std::shared_ptr<Texture<Spectrum>> &k,
                          const std::shared_ptr<Texture<double>> &uRoughness,
                          const std::shared_ptr<Texture<double>> &vRoughness,
                          const std::string &slopeDistribution = "beckmann",
                          const std::string &heightDistribution = "gaussian",
                          int scatteringOrder = 4,
                          const std::shared_ptr<Texture<double>> &bumpMap = nullptr,
                          bool remapRoughness = false);

    explicit MicrosurfaceConductor(RenderParams &params);

    void setScatterFuncs(SurfaceInteraction *isect,
                         MemoryArena &arena) const override;

private:
    // Private fields
    std::shared_ptr<Texture<Spectrum>> eta_;
    std::shared_ptr<Texture<Spectrum>> k_;
    std::shared_ptr<Texture<double>> uRoughness_, vRoughness_;
    std::string slopeDistribution_, heightDistribution_;
    int scatteringOrder_;
    std::shared_ptr<Texture<double>> bumpMap_;
    bool remapRoughness_;
};

SPICA_EXPORT_PLUGIN(MicrosurfaceConductor, "Microsurface conductor");

}  // namespace spica

#endif  // _SPICA_MS_SMITH_H_
