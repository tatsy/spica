#define SPICA_API_EXPORT
#include "translucent.h"

#include "core/memory.h"
#include "render/interaction.h"
#include "render/bsdf.h"
#include "render/fresnel.h"
#include "render/microfacet.h"
#include "render/texture.h"

namespace spica {

TranslucentMaterial::TranslucentMaterial(const std::shared_ptr<Texture<Spectrum>>& Kd,
                                         const std::shared_ptr<Texture<Spectrum>>& Ks,
                                         const std::shared_ptr<Texture<Spectrum>>& Kr,
                                         const std::shared_ptr<Texture<Spectrum>>& Kt,
                                         const std::shared_ptr<Texture<double>>& rough,
                                         const std::shared_ptr<Texture<double>>& bumpMap,
                                         bool remapRoughness)
    : Kd_{ Kd }
    , Ks_{ Ks }
    , Kr_{ Kr }
    , Kt_{ Kt }
    , roughness_{ rough }
    , bumpMap_{ bumpMap }
    , remapRoughness_{ remapRoughness } {
}

void TranslucentMaterial::setScatterFuncs(SurfaceInteraction* isect,
                                          MemoryArena& arena) const {
    // if (bumpMap_) bump(bumpMap_, isect);

    // TODO: Why constant eta??    
    const double eta = 1.5;
    isect->setBSDF(arena.allocate<BSDF>(*isect, eta));

    Spectrum re = Spectrum::clamp(Kr_->evaluate(*isect));
    Spectrum tr = Spectrum::clamp(Kt_->evaluate(*isect));
    if (re.isBlack() && tr.isBlack()) return;

    Spectrum di = Spectrum::clamp(Kd_->evaluate(*isect));
    if (!di.isBlack()) {
        if (!re.isBlack()) {
            isect->bsdf()->add(arena.allocate<LambertianReflection>(re * di));
        }
        if (!tr.isBlack()) {
            isect->bsdf()->add(arena.allocate<LambertianTransmission>(tr * di));
        }
    }

    Spectrum sp = Spectrum::clamp(Ks_->evaluate(*isect));
    if (!sp.isBlack() && (!re.isBlack() || !tr.isBlack())) {
        double rough = !roughness_ ? 0.0 : roughness_->evaluate(*isect);
        if (remapRoughness_) {
            rough = TrowbridgeReitzDistribution::roughnessToAlpha(rough);
        }

        MicrofacetDistribution* distrib = 
            arena.allocate<TrowbridgeReitzDistribution>(rough, rough);

        if (!re.isBlack()) {
            Fresnel* fresnel = arena.allocate<FresnelDielectric>(1.0, eta);        
            isect->bsdf()->add(arena.allocate<MicrofacetReflection>(
                re * sp, distrib, fresnel));
        }
        if (!tr.isBlack()) {
            isect->bsdf()->add(arena.allocate<MicrofacetTransmission>(
                tr * sp, distrib, 1.0, eta));
        }
    }
}

}  // namespace spica
