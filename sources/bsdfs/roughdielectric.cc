#define SPICA_API_EXPORT
#include "roughdielectric.h"

#include "core/memory.h"
#include "core/interaction.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/microfacet.h"
#include "core/texture.h"

namespace spica {

RoughDielectric::RoughDielectric(const std::shared_ptr<Texture<Spectrum>>& Kr,
                                 const std::shared_ptr<Texture<Spectrum>>& Kt,
                                 const std::shared_ptr<Texture<double>>& uRoughness,
                                 const std::shared_ptr<Texture<double>>& vRoughness,
                                 const std::shared_ptr<Texture<double>>& index,
                                 const std::shared_ptr<Texture<double>>& bumpMap,
                                 bool remapRoughness)
    : Kr_{ Kr }
    , Kt_{ Kt }
    , uRoughness_{ uRoughness }
    , vRoughness_{ vRoughness }
    , index_{ index }
    , bumpMap_{ bumpMap }
    , remapRoughness_{ remapRoughness } {
}

RoughDielectric::RoughDielectric(RenderParams &params)
    : RoughDielectric{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularReflectance")),
                      std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularTransmittance")),
                      std::static_pointer_cast<Texture<double>>(params.getTexture("alpha")),
                      std::static_pointer_cast<Texture<double>>(params.getTexture("alpha")),
                      std::static_pointer_cast<Texture<double>>(params.getTexture("intIOR")),
                      std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void RoughDielectric::setScatterFuncs(SurfaceInteraction* isect,
                                    MemoryArena& arena) const {
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
            uRough = TrowbridgeReitzDistribution::roughnessToAlpha(uRough);   
            vRough = TrowbridgeReitzDistribution::roughnessToAlpha(vRough);
        }

        MicrofacetDistribution* distrib = 
            isSpecular ? nullptr
                       : arena.allocate<TrowbridgeReitzDistribution>(
                           uRough, vRough);

        if (!re.isBlack()) {
            Fresnel* fresnel = arena.allocate<FresnelDielectric>(1.0, eta);
            if (isSpecular) {
                isect->bsdf()->add(arena.allocate<SpecularReflection>(re, fresnel));
            } else {
                isect->bsdf()->add(arena.allocate<MicrofacetReflection>(re, distrib, fresnel));
            }
        }

        if (!tr.isBlack()) {
            if (isSpecular) {
                isect->bsdf()->add(arena.allocate<SpecularTransmission>(tr, 1.0, eta));
            } else {
                isect->bsdf()->add(arena.allocate<MicrofacetTransmission>(tr, distrib, 1.0, eta));
            }
        }
    }
}

} // namespace spica
