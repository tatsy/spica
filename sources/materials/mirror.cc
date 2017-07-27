#define SPICA_API_EXPORT
#include "mirror.h"

#include "core/memory.h"
#include "core/interaction.h"

#include "core/texture.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/fresnel.h"

namespace spica {

MirrorMaterial::MirrorMaterial(const std::shared_ptr<Texture<Spectrum>>& Kr,
                               const std::shared_ptr<Texture<double>>& bump)
    : Kr_{ Kr }
    , bumpMap_{ bump } {
}

void MirrorMaterial::setScatterFuncs(SurfaceInteraction* intr,
                                     MemoryArena& arena) const {
    // if (bumpMap_) bump(intr, bumpMap_);

    intr->setBSDF(arena.allocate<BSDF>(*intr));
    Spectrum r = Spectrum::clamp(Kr_->evaluate(*intr));
    if (!r.isBlack()) {
        Fresnel* fresnel = arena.allocate<FresnelNoOp>();
        intr->bsdf()->add(arena.allocate<SpecularReflection>(r, fresnel));
    }
}

}  // namespace spica
