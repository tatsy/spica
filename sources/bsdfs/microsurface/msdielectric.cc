#define SPICA_API_EXPORT
#include "msdielectric.h"

#include <memory>

#include "core/memory.h"
#include "core/interaction.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/texture.h"
#include "core/random.h"
#include "core/microfacet.h"

#include "microsurface.h"

namespace spica {

MicrosurfaceDielectric::MicrosurfaceDielectric(const std::shared_ptr<Texture<Spectrum>>& Kr,
                                               const std::shared_ptr<Texture<Spectrum>>& Kt,
                                               const std::shared_ptr<Texture<double>>& uRoughness,
                                               const std::shared_ptr<Texture<double>>& vRoughness,
                                               const std::shared_ptr<Texture<double>>& index,
                                               const std::string &slopeDistribution,
                                               const std::string &heightDistribution,
                                               const std::shared_ptr<Texture<double>>& bumpMap,
                                               bool remapRoughness)
    : Kr_{Kr}
    , Kt_{Kr}
    , uRoughness_{uRoughness}
    , vRoughness_{vRoughness}
    , index_{index}
    , slopeDistribution_{slopeDistribution}
    , heightDistribution_{heightDistribution}
    , bumpMap_{bumpMap}
    , remapRoughness_{remapRoughness} {
}

MicrosurfaceDielectric::MicrosurfaceDielectric(RenderParams &params)
    : MicrosurfaceDielectric{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularReflectance")),
                             std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularTransmittance")),
                             std::static_pointer_cast<Texture<double>>(params.getTexture("alpha")),
                             std::static_pointer_cast<Texture<double>>(params.getTexture("alpha")),
                             std::static_pointer_cast<Texture<double>>(params.getTexture("intIOR")),
                             params.getString("distribution", "beckmann", true),
                             params.getString("heightDistribution", "gaussian", true),
                             std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void MicrosurfaceDielectric::setScatterFuncs(SurfaceInteraction *isect,
                                             MemoryArena &arena) const {
    if (bumpMap_) bump(isect, bumpMap_);

    double eta = index_->evaluate(*isect);
    double uRough = uRoughness_->evaluate(*isect);
    double vRough = vRoughness_->evaluate(*isect);
    Spectrum re = Spectrum::clamp(Kr_->evaluate(*isect));
    Spectrum tr = Spectrum::clamp(Kt_->evaluate(*isect));

    isect->setBSDF(arena.allocate<BSDF>(*isect, eta));
    if (re.isBlack() && tr.isBlack()) return;

    bool isSpecular = uRough == 0 && vRough == 0;
    if (isSpecular) {
        // Specular reflection
        isect->bsdf()->add(arena.allocate<FresnelSpecular>(re, tr, 1.0, eta));
    } else {
        // Non-specular reflection
        if (remapRoughness_) {
            if (slopeDistribution_ == "beckmann") {
                uRough = BeckmannDistribution::roughnessToAlpha(uRough);
                uRough = BeckmannDistribution::roughnessToAlpha(vRough);
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

        isect->bsdf()->add(arena.allocate<MicrosurfaceScattering>(re, tr, 1.0, eta, hDist, sDist));
    }
}

}  // namespace spica
