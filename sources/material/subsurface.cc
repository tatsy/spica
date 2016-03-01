#define SPICA_API_EXPORT
#include "subsurface.h"

#include "../core/memory.h"
#include "../bxdf/bsdf.h"
#include "../bxdf/bssrdf.h"
#include "../bxdf/fresnel.h"
#include "../bxdf/microfacet.h"
#include "../texture/texture.h"

namespace spica {

SubsurfaceMaterial::SubsurfaceMaterial(double scale,
                                       const std::shared_ptr<Texture<Spectrum>>& Kr,
                                       const std::shared_ptr<Texture<Spectrum>>& Kt,
                                       const std::shared_ptr<Texture<Spectrum>>& sigmaAbsorb,
                                       const std::shared_ptr<Texture<Spectrum>>& sigmaScatter,
                                       double g, double eta,
                                       const std::shared_ptr<Texture<double>>& uRoughness,
                                       const std::shared_ptr<Texture<double>>& vRoughness,
                                       const std::shared_ptr<Texture<double>>& bumpMap,
                                       bool remapRoughness)
    : scale_{ scale }
    , Kr_{ Kr }
    , Kt_{ Kt }
    , sigmaAbsorb_{ sigmaAbsorb }
    , sigmaScatter_{ sigmaScatter }
    , eta_{ eta }
    , uRoughness_{ uRoughness }
    , vRoughness_{ vRoughness }
    , bumpMap_{ bumpMap }
    , remapRoughness_{ remapRoughness }
    , table_{} {
    computeBeamDiffusionBSSRDF(g, eta, &table_);
}

void SubsurfaceMaterial::setScatterFuncs(SurfaceInteraction* isect,
                                         MemoryArena& arena) const {
    // if (bumpMap_) bump(bumpMap_, isect);

    const Spectrum re = Spectrum::clamp(Kr_->evaluate(*isect));
    const Spectrum tr = Spectrum::clamp(Kt_->evaluate(*isect));
    double uRough = uRoughness_ ? uRoughness_->evaluate(*isect) : 0.0;
    double vRough = vRoughness_ ? vRoughness_->evaluate(*isect) : 0.0;

    // TODO: check which method "allocate" is called.
    isect->setBSDF(arena.allocate<BSDF>(*isect, eta_));
    
    if (re.isBlack() && tr.isBlack()) return;

    bool isSpecular = uRough == 0.0 && vRough == 0.0;
    if (isSpecular) {
        // Specular reflection
        isect->bsdf()->add(arena.allocate<FresnelSpecular>(re, tr, 1.0, eta_));
    } else {
        // Non-specular reflection
        if (remapRoughness_) {
            uRough = TrowbridgeReitzDistribution::roughnessToAlpha(uRough);
            vRough = TrowbridgeReitzDistribution::roughnessToAlpha(vRough);
        }

        MicrofacetDistribution* distrib =
            isSpecular ? nullptr
                       : arena.allocate<TrowbridgeReitzDistribution>(
                             uRough, vRough);

        if (!re.isBlack()) {
            Fresnel* fresnel = arena.allocate<FresnelDielectric>(1.0, eta_);
            if (isSpecular) {
                isect->bsdf()->add(arena.allocate<SpecularReflection>(
                    re, fresnel));
            } else {
                isect->bsdf()->add(arena.allocate<MicrofacetReflection>(re,
                    distrib, fresnel));
            }
        }

        if (!tr.isBlack()) {
            if (isSpecular) {
                isect->bsdf()->add(
                    arena.allocate<SpecularTransmission>(tr, 1.0, eta_));
            } else {
                isect->bsdf()->add(
                    arena.allocate<MicrofacetTransmission>(tr, distrib, 1.0, eta_));
            }
        }
    }
    Spectrum sigA = scale_ * Spectrum::clamp(sigmaAbsorb_->evaluate(*isect));
    Spectrum sigS = scale_ * Spectrum::clamp(sigmaScatter_->evaluate(*isect));
    isect->setBSSRDF(arena.allocate<DiffuseBSSRDF>(*isect, this, eta_,
                                                   sigA, sigS, table_));
}

}  // namespace spica
