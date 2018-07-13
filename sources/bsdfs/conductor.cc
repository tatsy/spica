#define SPICA_API_EXPORT
#include "conductor.h"

#include "core/memory.h"
#include "core/interaction.h"

#include "core/texture.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/fresnel.h"

namespace spica {

Conductor::Conductor(const std::shared_ptr<Texture<Spectrum>>& eta,
                     const std::shared_ptr<Texture<Spectrum>> &k,
                     const std::shared_ptr<Texture<double>>& bump)
    : eta_{eta}
    , k_{k}
    , bumpMap_{ bump } {
}

Conductor::Conductor(RenderParams &params)
    : Conductor{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("eta")),
                std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("k")),
                std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void Conductor::setScatterFuncs(SurfaceInteraction* isect,
                                MemoryArena& arena) const {
    if (bumpMap_) bump(isect, bumpMap_);

    isect->setBSDF(arena.allocate<BSDF>(*isect));

    const Spectrum eta = eta_->evaluate(*isect);
    const Spectrum k = k_->evaluate(*isect);
    Fresnel* fresnel = arena.allocate<FresnelConductor>(Spectrum(1.0), eta, k);
    isect->bsdf()->add(arena.allocate<SpecularReflection>(Spectrum(1.0), fresnel));
}

}  // namespace spica
