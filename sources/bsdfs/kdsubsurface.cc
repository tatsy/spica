#define SPICA_API_EXPORT
#include "kdsubsurface.h"

#include "core/memory.h"

#include "core/bsdf.h"
#include "core/bssrdf.h"
#include "core/fresnel.h"
#include "core/microfacet.h"
#include "core/texture.h"

namespace spica {

KdSubsurfaceMaterial::KdSubsurfaceMaterial(double scale,
                                           const std::shared_ptr<Texture<Spectrum>>& Kd,
                                           const std::shared_ptr<Texture<Spectrum>>& Kr,
                                           const std::shared_ptr<Texture<Spectrum>>& Kt,
                                           const std::shared_ptr<Texture<Spectrum>>& mfp,
                                           double g, double eta,
                                           const std::shared_ptr<Texture<double>>& uRoughness,
                                           const std::shared_ptr<Texture<double>>& vRoughness,
                                           const std::shared_ptr<Texture<double>>& bumpMap,
                                           bool remapRoughness)
    : scale_{ scale }
    , Kd_{ Kd }
    , Kr_{ Kr }
    , Kt_{ Kt }
    , mfp_{ mfp }
    , uRoughness_{ uRoughness }
    , vRoughness_{ vRoughness }
    , bumpMap_{ bumpMap }
    , eta_{ eta }
    , remapRoughness_{ remapRoughness }
    , table_{} {
    computeBeamDiffusionBSSRDF(g, eta, &table_);
}

void KdSubsurfaceMaterial::setScatterFuncs(SurfaceInteraction* isect,
                                           MemoryArena& arena) const {
    // if (bumpMap_) bump(bumpMap_, isect);    
    
    Spectrum re = Spectrum::clamp(Kr_->evaluate(*isect));
    Spectrum tr = Spectrum::clamp(Kt_->evaluate(*isect));
    double uRough = uRoughness_ ? uRoughness_->evaluate(*isect) : 0.0;
    double vRough = vRoughness_ ? vRoughness_->evaluate(*isect) : 0.0;

    isect->setBSDF(arena.allocate<BSDF>(*isect, eta_));

    if (re.isBlack() && tr.isBlack()) return;

    bool isSpecular = uRough == 0.0 && vRough == 0.0;
    if (isSpecular) {
        isect->bsdf()->add(arena.allocate<FresnelSpecular>(re, tr, 1.0, eta_));
    } else {
        if (remapRoughness_) {
            uRough = TrowbridgeReitzDistribution::roughnessToAlpha(uRough);
            vRough = TrowbridgeReitzDistribution::roughnessToAlpha(vRough);
        }

        MicrofacetDistribution* distrib =
            isSpecular ? nullptr
                       : arena.allocate<TrowbridgeReitzDistribution>(uRough, vRough); 

        if (!re.isBlack()) {
            Fresnel* fresnel = arena.allocate<FresnelDielectric>(1.0, eta_);
            if (isSpecular) {
                isect->bsdf()->add(arena.allocate<SpecularReflection>(re, fresnel));
            } else {
                isect->bsdf()->add(arena.allocate<MicrofacetReflection>(re, distrib, fresnel));
            }
        }

        if (!tr.isBlack()) {
            if (isSpecular) {
                isect->bsdf()->add(arena.allocate<SpecularTransmission>(tr, 1.0, eta_));
            } else {
                isect->bsdf()->add(arena.allocate<MicrofacetTransmission>(tr, distrib, 1.0, eta_));
            }
        }
    }

    Spectrum mfree = scale_ * Spectrum::clamp(mfp_->evaluate(*isect));
    Spectrum di = Spectrum::clamp(Kd_->evaluate(*isect));
    Spectrum sigA, sigS;
    subsurfaceFromDiffuse(table_, di, mfree, &sigA, &sigS);
    isect->setBSSRDF(arena.allocate<DiffuseBSSRDF>(*isect, this, eta_, sigA, sigS, table_));
}

}  // namespace spica
