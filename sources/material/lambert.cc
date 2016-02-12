#define SPICA_API_EXPORT
#include "lambert.h"

#include "../core/memory.h"
#include "../core/interaction.h"

namespace spica {

LambertianMaterial::LambertianMaterial(const std::shared_ptr<Texture<Spectrum>>& Kd,
                                       const std::shared_ptr<Texture<double>>& bumpMap)
    : Kd_{ Kd }
    , bumpMap_{ bumpMap } {
}

void LambertianMaterial::setScatterFuncs(SurfaceInteraction* intr,
                                         MemoryArena& arena) const {
    if (bumpMap_) bump(intr, bumpMap_);

    Spectrum r = Spectrum::clamp(Kd_->evaluate(*intr));
    intr->bsdf().reset(arena.allocate<LambertianBRDF>(r));
}

}  // namespace
