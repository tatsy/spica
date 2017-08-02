#define SPICA_API_EXPORT
#include "dipole.h"

#include "core/memory.h"
#include "core/interaction.h"
#include "core/bsdf.h"
#include "core/bssrdf.h"
#include "core/fresnel.h"
#include "core/microfacet.h"
#include "core/texture.h"

namespace spica {

Dipole::Dipole(double scale,
               const std::shared_ptr<Texture<Spectrum>>& sigmaAbsorb,
               const std::shared_ptr<Texture<Spectrum>>& sigmaScatter,
               double g, double eta)
    : scale_{ scale }
    , sigmaAbsorb_{ sigmaAbsorb }
    , sigmaScatter_{ sigmaScatter }
    , eta_{ eta }
    , table_{} {
    computeBeamDiffusionBSSRDF(g, eta, &table_);
}

Dipole::Dipole(RenderParams &params)
    : Dipole{params.getDouble("scale", 1.0, true),
             std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("sigmaA", true)),
             std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("sigmaS", true)),
             params.getDouble("g", 0.0, true),
             params.getDouble("intIOR", true)} {
}

void Dipole::setScatterFuncs(SurfaceInteraction* isect,
                             MemoryArena& arena) const {
    Spectrum sigA = scale_ * Spectrum::clamp(sigmaAbsorb_->evaluate(*isect));
    Spectrum sigS = scale_ * Spectrum::clamp(sigmaScatter_->evaluate(*isect));
    isect->setBSSRDF(arena.allocate<DiffuseBSSRDF>(*isect, this, eta_,
                                                   sigA, sigS, table_));
}

}  // namespace spica
