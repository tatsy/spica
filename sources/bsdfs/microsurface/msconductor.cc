#define SPICA_API_EXPORT
#include "msconductor.h"

#include <memory>

#include "core/memory.h"
#include "core/interaction.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/microfacet.h"
#include "core/texture.h"
#include "core/random.h"

#include "microsurface.h"

namespace spica {

MicrosurfaceConductor::MicrosurfaceConductor(const std::shared_ptr<Texture<Spectrum>>& eta,
                                             const std::shared_ptr<Texture<Spectrum>>& k,
                                             const std::shared_ptr<Texture<double>>& uRoughness,
                                             const std::shared_ptr<Texture<double>>& vRoughness,
                                             const std::string &slopeDistribution,
                                             const std::string &heightDistribution,
                                             const std::shared_ptr<Texture<double>>& bumpMap,
                                             bool remapRoughness)
    : eta_{eta}
    , k_{k}
    , uRoughness_{uRoughness}
    , vRoughness_{vRoughness}
    , slopeDistribution_{slopeDistribution}
    , heightDistribution_{heightDistribution}
    , bumpMap_{bumpMap}
    , remapRoughness_{remapRoughness} {
}

MicrosurfaceConductor::MicrosurfaceConductor(RenderParams &params)
    : MicrosurfaceConductor{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("eta", true)),
                            std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("k")),
                            std::static_pointer_cast<Texture<double>>(params.getTexture("alpha")),
                            std::static_pointer_cast<Texture<double>>(params.getTexture("alpha")),
                            params.getString("slopeDistribution", "beckmann", true),
                            params.getString("heightDistribution", "gaussian", true),
                            std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void MicrosurfaceConductor::setScatterFuncs(SurfaceInteraction *isect,
                                            MemoryArena &arena) const {
    if (bumpMap_) bump(isect, bumpMap_);

    Spectrum eta = eta_->evaluate(*isect);
    Spectrum k = k_->evaluate(*isect);
    double uRough = uRoughness_->evaluate(*isect);
    double vRough = vRoughness_->evaluate(*isect);

    isect->setBSDF(arena.allocate<BSDF>(*isect));

    Fresnel *fresnel = arena.allocate<FresnelConductor>(Spectrum(1.0), eta, k);

    bool isSpecular = uRough == 0 && vRough == 0;
    if (isSpecular) {
        // Specular reflection
        isect->bsdf()->add(arena.allocate<SpecularReflection>(Spectrum(1.0), fresnel));
    } else {
        // Non-specular reflection
        if (remapRoughness_) {
            if (slopeDistribution_ == "beckmann") {
                uRough = BeckmannDistribution::roughnessToAlpha(uRough);
                vRough = BeckmannDistribution::roughnessToAlpha(vRough);
            } else if (slopeDistribution_ == "ggx") {
                uRough = TrowbridgeReitzDistribution::roughnessToAlpha(uRough);   
                vRough = TrowbridgeReitzDistribution::roughnessToAlpha(vRough);
            }
        }

        MicrosurfaceHeight *hDist = nullptr;
        if (heightDistribution_ == "uniform") {
            hDist = arena.allocate<MicrosurfaceUniformHeight>();
        } else if (heightDistribution_ == "gaussian") {
            hDist = arena.allocate<MicrosurfaceGaussianHeight>();
        }

        MicrosurfaceSlope *sDist = nullptr;
        if (slopeDistribution_ == "beckmann") {
            sDist = arena.allocate<MicrosurfaceBeckmannSlope>(uRough, vRough);
        } else if (slopeDistribution_ == "ggx") {
            sDist = arena.allocate<MicrosurfaceTrowbridgeReitzSlope>(uRough, vRough);
        }

        isect->bsdf()->add(arena.allocate<MicrosurfaceReflection>(Spectrum(1.0), fresnel, hDist, sDist));
    }
}

}  // namespace spica
