#define SPICA_API_EXPORT
#include "conductor.h"

#include "core/memory.h"
#include "core/interaction.h"

#include "core/texture.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/fresnel.h"

namespace spica {

Conductor::Conductor(const std::shared_ptr<Texture<Spectrum>>& Kr,
                     const std::shared_ptr<Texture<double>>& bump)
    : Kr_{ Kr }
    , bumpMap_{ bump } {
}

Conductor::Conductor(RenderParams &params)
    : Conductor{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularReflectance")),
                std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void Conductor::setScatterFuncs(SurfaceInteraction* intr,
                                MemoryArena& arena) const {
    if (bumpMap_) bump(intr, bumpMap_);

    intr->setBSDF(arena.allocate<BSDF>(*intr));
    Spectrum r = Spectrum::clamp(Kr_->evaluate(*intr));
    if (!r.isBlack()) {
        Fresnel* fresnel = arena.allocate<FresnelNoOp>();
        intr->bsdf()->add(arena.allocate<SpecularReflection>(r, fresnel));
    }
}

}  // namespace spica
