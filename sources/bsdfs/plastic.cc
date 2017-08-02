#define SPICA_API_EXPORT
#include "plastic.h"

#include "core/memory.h"
#include "core/interaction.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/fresnel.h"
#include "core/microfacet.h"
#include "core/texture.h"

namespace spica {

Plastic::Plastic(const std::shared_ptr<Texture<Spectrum>>& Kd,
                 const std::shared_ptr<Texture<Spectrum>>& Ks,
                 const std::shared_ptr<Texture<double>>& bumpMap)
    : SurfaceMaterial{}
    , Kd_{ Kd }
    , Ks_{ Ks }
    , bumpMap_{ bumpMap } {
}

Plastic::Plastic(RenderParams &params)
    : Plastic{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("diffuseReflectance")),
              std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularReflectance")),
              std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void Plastic::setScatterFuncs(SurfaceInteraction* isect,
                              MemoryArena& arena) const {
    if (bumpMap_) bump(isect, bumpMap_);

    isect->setBSDF(arena.allocate<BSDF>(*isect));
    
    Spectrum kd = Kd_->evaluate(*isect);
    if (!kd.isBlack()) {
        isect->bsdf()->add(arena.allocate<LambertianReflection>(kd));
    }

    Spectrum ks = Ks_->evaluate(*isect);
    if (!ks.isBlack()) {
        Fresnel* fresnel = arena.allocate<FresnelNoOp>();
        isect->bsdf()->add(arena.allocate<SpecularReflection>(ks, fresnel));
    }
}

}  // namespace spica
