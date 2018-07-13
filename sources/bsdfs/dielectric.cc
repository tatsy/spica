#define SPICA_API_EXPORT
#include "dielectric.h"

#include "core/memory.h"
#include "core/interaction.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/microfacet.h"
#include "core/texture.h"

namespace spica {

Dielectric::Dielectric(const std::shared_ptr<Texture<Spectrum>>& Kr,
                       const std::shared_ptr<Texture<Spectrum>>& Kt,
                       const std::shared_ptr<Texture<double>>& index,
                       const std::shared_ptr<Texture<double>>& bumpMap)
    : Kr_{ Kr }
    , Kt_{ Kt }
    , index_{ index }
    , bumpMap_{ bumpMap } {
}

Dielectric::Dielectric(RenderParams &params)
    : Dielectric{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularReflectance")),
                 std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularTransmittance")),
                 std::static_pointer_cast<Texture<double>>(params.getTexture("intIOR", 1.333)),
                 std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void Dielectric::setScatterFuncs(SurfaceInteraction* isect,
                                    MemoryArena& arena) const {
    if (bumpMap_) bump(isect, bumpMap_);

    double eta = index_->evaluate(*isect);
    Spectrum re = Spectrum::clamp(Kr_->evaluate(*isect));
    Spectrum tr = Spectrum::clamp(Kt_->evaluate(*isect));

    isect->setBSDF(arena.allocate<BSDF>(*isect, eta));
    if (re.isBlack() && tr.isBlack()) return;

    isect->bsdf()->add(arena.allocate<FresnelSpecular>(re, tr, 1.0, eta));
}

} // namespace spica
