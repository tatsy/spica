#define SPICA_API_EXPORT
#include "glass.h"

#include "core/memory.h"
#include "render/interaction.h"
#include "render/bxdf.h"
#include "render/bsdf.h"
#include "render/microfacet.h"
#include "render/texture.h"

namespace spica {

GlassMaterial::GlassMaterial(const std::shared_ptr<Texture<Spectrum>>& Kr,
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

void GlassMaterial::setScatterFuncs(SurfaceInteraction* isect,
                                    MemoryArena& arena) const {
    // if (bumpMap_) bump(bumpMap_, isect);

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
                isect->bsdf()->add(arena.allocate<MicrofacetReflection>(re,
                    distrib, fresnel));
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
