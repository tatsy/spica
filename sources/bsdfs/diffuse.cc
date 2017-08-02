#define SPICA_API_EXPORT
#include "diffuse.h"

#include "core/memory.h"
#include "core/interaction.h"
#include "core/bsdf.h"
#include "core/bxdf.h"
#include "core/texture.h"

namespace spica {

Diffuse::Diffuse(const std::shared_ptr<Texture<Spectrum>>& Kd,
                 const std::shared_ptr<Texture<double>>& bumpMap)
    : Kd_{ Kd }
    , bumpMap_{ bumpMap } {
}

Diffuse::Diffuse(RenderParams &params)
    : Diffuse{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("reflectance", true)),
              std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap", true))} {
}

void Diffuse::setScatterFuncs(SurfaceInteraction* intr,
                              MemoryArena& arena) const {
    if (bumpMap_) bump(intr, bumpMap_);

    intr->setBSDF(arena.allocate<BSDF>(*intr));
    Spectrum r = Spectrum::clamp(Kd_->evaluate(*intr));
    if (!r.isBlack()) {
        intr->bsdf()->add(arena.allocate<LambertianReflection>(r));
    }
}

}  // namespace
