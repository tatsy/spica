#define SPICA_API_EXPORT
#include "plastic.h"

#include "core/memory.h"
#include "render/interaction.h"
#include "render/bxdf.h"
#include "render/bsdf.h"
#include "render/fresnel.h"
#include "render/microfacet.h"
#include "render/texture.h"

namespace spica {

PlasticMaterial::PlasticMaterial(const std::shared_ptr<Texture<Spectrum>>& Kd,
                                 const std::shared_ptr<Texture<Spectrum>>& Ks,
                                 const std::shared_ptr<Texture<double>>& roughness,
                                 const std::shared_ptr<Texture<double>>& bumpMap,
                                 bool remapRoughness)
    : Material{}
    , Kd_{ Kd }
    , Ks_{ Ks }
    , roughness_{ roughness }
    , bumpMap_{ bumpMap }
    , remapRoughness_{ remapRoughness } {
}

void PlasticMaterial::setScatterFuncs(SurfaceInteraction* isect,
                                      MemoryArena& arena) const {
    // if (bumpMap_) bump(bumpMap_, isect);

    isect->setBSDF(arena.allocate<BSDF>(*isect));
    
    Spectrum kd = Kd_->evaluate(*isect);
    if (!kd.isBlack()) {
        isect->bsdf()->add(arena.allocate<LambertianReflection>(kd));
    }

    Spectrum ks = Ks_->evaluate(*isect);
    if (!ks.isBlack()) {
        Fresnel* fresnel = arena.allocate<FresnelDielectric>(1.5, 1.0);
        double rough = roughness_ ? roughness_->evaluate(*isect) : 0.0;
        if (remapRoughness_) {
            rough = TrowbridgeReitzDistribution::roughnessToAlpha(rough);
        }

        MicrofacetDistribution* distrib =
            arena.allocate<TrowbridgeReitzDistribution>(rough, rough);
        isect->bsdf()->add(arena.allocate<MicrofacetReflection>(ks, distrib, fresnel));
    }
}

}  // namespace spica
