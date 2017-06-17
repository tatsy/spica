#define SPICA_API_EXPORT
#include "roughconductor.h"

#include "core/memory.h"
#include "core/interaction.h"
#include "core/bsdf.h"
#include "core/fresnel.h"
#include "core/microfacet.h"
#include "core/texture.h"

namespace spica {

RoughConductor::RoughConductor(const std::shared_ptr<Texture<Spectrum>>& eta,
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

RoughConductor::RoughConductor(RenderParams &params)
    : RoughConductor{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("eta", true)),
                     std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularReflectance", true)),
                     std::static_pointer_cast<Texture<double>>(params.getTexture("alpha", true)),
                     std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap", true))} {
}
    
void RoughConductor::setScatterFuncs(SurfaceInteraction* isect,
                                    MemoryArena& arena) const {
    if (bumpMap_) bump(isect, bumpMap_);

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
