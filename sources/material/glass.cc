#define SPICA_API_EXPORT
#include "glass.h"

#include "../core/memory.h"
#include "../core/interaction.h"
#include "../bxdf/bsdf.h"
#include "../bxdf/bxdf.h"
#include "../texture/texture.h"

namespace spica {

GlassMaterial::GlassMaterial(const std::shared_ptr<Texture<Spectrum>>& Kr,
                             const std::shared_ptr<Texture<Spectrum>>& Kt,
                             const std::shared_ptr<Texture<double>>& uRoughness,
                             const std::shared_ptr<Texture<double>>& vRoughness,
                             const std::shared_ptr<Texture<double>>& index,
                             const std::shared_ptr<Texture<double>>& bumpMap,
                             bool remapRoughness)
    : Kr_{ Kr }
    , Kt_{ Kt }
    , uRoughness_{ uRoughness }
    , vRoughness_{ vRoughness }
    , index_{ index }
    , bumpMap_{ bumpMap }
    , remapRoughness_{ remapRoughness } {
}

void GlassMaterial::setScatterFuncs(SurfaceInteraction* isect,
                                    MemoryArena& arena) const {
    // if (bumpMap_) bump(bumpMap_, isect);

    double eta = index_->evaluate(*isect);
    double uRough = uRoughness_->evaluate(*isect);
    double vRough = vRoughness_->evaluate(*isect);
    Spectrum re = Spectrum::clamp(Kr_->evaluate(*isect));
    Spectrum tr = Spectrum::clamp(Kt_->evaluate(*isect));

    isect->setBSDF(arena.allocate<BSDF>(*isect, eta));
    if (re.isBlack() && tr.isBlack()) return;

    bool isSpecular = uRough == 0 && vRough == 0;
    if (isSpecular) {
        isect->bsdf()->add(arena.allocate<FresnelSpecular>(re, tr, 1.0, eta));
    } else {
        // TODO: Implement!!
    }
}

} // namespace spica
