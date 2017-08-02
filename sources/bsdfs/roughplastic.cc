#define SPICA_API_EXPORT
#include "roughplastic.h"

#include "core/memory.h"
#include "core/interaction.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/fresnel.h"
#include "core/microfacet.h"
#include "core/texture.h"

namespace spica {

RoughPlastic::RoughPlastic(const std::shared_ptr<Texture<Spectrum>>& Kd,
                           const std::shared_ptr<Texture<Spectrum>>& Ks,
                           const std::shared_ptr<Texture<double>>& roughness,
                           const std::shared_ptr<Texture<double>>& bumpMap,
                           bool remapRoughness)
    : SurfaceMaterial{}
    , Kd_{ Kd }
    , Ks_{ Ks }
    , roughness_{ roughness }
    , bumpMap_{ bumpMap }
    , remapRoughness_{ remapRoughness } {
}

RoughPlastic::RoughPlastic(RenderParams &params)
    : RoughPlastic{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("diffuseReflectance")),
                   std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularReflectance")),
                   std::static_pointer_cast<Texture<double>>(params.getTexture("alpha")),
                   std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void RoughPlastic::setScatterFuncs(SurfaceInteraction* isect,
                                      MemoryArena& arena) const {
    if (bumpMap_) bump(isect, bumpMap_);

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
