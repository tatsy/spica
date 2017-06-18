#define SPICA_API_EXPORT
#include "metal.h"

#include "../core/memory.h"
#include "../core/interaction.h"

#include "../bxdf/bsdf.h"
#include "../bxdf/fresnel.h"
#include "../bxdf/microfacet.h"

#include "../texture/texture.h"

namespace spica {

MetalMaterial::MetalMaterial(const std::shared_ptr<Texture<Spectrum>>& eta,
                  const std::shared_ptr<Texture<Spectrum>>& k,
                  const std::shared_ptr<Texture<double>>& roughness,
                  const std::shared_ptr<Texture<double>>& bump,
                  bool remapRoughness) 
    : eta_{ eta }
    , k_{ k }
    , roughness_{ roughness }
    , bumpMap_{ bump }
    , remapRoughness_{ remapRoughness } {
}

void MetalMaterial::setScatterFuncs(SurfaceInteraction* isect,
                                    MemoryArena& arena) const {
    // if (bumpMap_) bump(bumpMap_, isect);
    isect->setBSDF(arena.allocate<BSDF>(*isect));

    double rough = roughness_->evaluate(*isect);
    if (remapRoughness_) {
        rough = TrowbridgeReitzDistribution::roughnessToAlpha(rough);    
    }

    Fresnel* frMf =
        arena.allocate<FresnelConductor>(Spectrum(1.0), eta_->evaluate(*isect),
                                         k_->evaluate(*isect));
    MicrofacetDistribution* distrib = 
        arena.allocate<TrowbridgeReitzDistribution>(rough, rough);
    isect->bsdf()->add(arena.allocate<MicrofacetReflection>(Spectrum(1.0), distrib, frMf));
}

}  // namespace spica
