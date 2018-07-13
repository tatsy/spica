#define SPICA_API_EXPORT
#include "roughplastic.h"

#include "core/memory.h"
#include "core/vect_math.h"
#include "core/interaction.h"
#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/random.h"
#include "core/fresnel.h"
#include "core/microfacet.h"
#include "core/texture.h"
#include "core/sampling.h"

namespace spica {

namespace {

static thread_local Random random;

class RoughPlasticBRDF : public BxDF {
public:
    RoughPlasticBRDF(const Spectrum &Kd, const Spectrum &Ks,
                     MicrofacetDistribution *distrib,
                     double etaA, double etaB)
        : BxDF{ BxDFType::Reflection | BxDFType::Diffuse | BxDFType::Glossy }
        , Kd_{ Kd }
        , Ks_{ Ks }
        , distrib_{ distrib }
        , etaA_{ etaA }
        , etaB_{ etaB } {
        specularSamplingWeight_ = Ks_.luminance() / (Kd_.luminance() + Ks_.luminance());
    }

    Spectrum f(const Vector3d& wo,
               const Vector3d& wi) const override {
        const double cosThetaO = vect::cosTheta(wo);
        const double cosThetaI = vect::cosTheta(wi);
        if (cosThetaO == 0.0 || cosThetaI == 0.0) return Spectrum(0.0);
        if (!vect::sameHemisphere(wo, wi)) return Spectrum(0.0);

        const Vector3d wh = vect::normalize(wi + wo);
        const double F = FrDielectric(vect::dot(wo, wh), etaA_, etaB_);
        Spectrum spec = Ks_ * F * distrib_->D(wh) * distrib_->G(wo, wi, wh) / (4.0 * cosThetaO * cosThetaI);
        Spectrum diff = Kd_ * (1.0 - F) * INV_PI;
        return spec + diff;
    }

    Spectrum sample(const Vector3d& wo, Vector3d* wi,
                    const Point2d& rands, double* pdf,
                    BxDFType* sampledType) const override {
        if (wo.z() == 0.0) return Spectrum(0.0);

        const Vector3d wh = distrib_->sample(wo, rands);
        const double Fo = FrDielectric(vect::dot(wo, wh), etaA_, etaB_);
        const double probSpecular = (Fo * specularSamplingWeight_) /
                                    (Fo * specularSamplingWeight_ + (1.0 - Fo) * (1.0 - specularSamplingWeight_));
        if (random.get1D() < probSpecular) {
            // Glossy reflection
            *wi = vect::reflect(wo, wh);
            if (!vect::sameHemisphere(wo, *wi)) {
                return Spectrum(0.0);
            }
            *pdf = probSpecular * distrib_->pdf(wo, wh) / (4.0 * vect::absDot(wo, wh));
            
            double cosThetaO = vect::cosTheta(wo);
            double cosThetaI = vect::cosTheta(*wi);
            return Ks_ * Fo * distrib_->D(wh) * distrib_->G(wo, *wi, wh) / (4.0 * cosThetaO * cosThetaI);
        } else {
            // Diffuse reflection
            *wi = sampleCosineHemisphere(rands);
            *pdf = (1.0 - probSpecular) * std::abs(vect::cosTheta(*wi)) * INV_PI;
            
            const double eta = etaB_ / etaA_;
            const double invEta = 1.0 / eta;
            double Fi = FrDielectric(vect::cosTheta(*wi), etaA_, etaB_);
            double Fdr = FrDiffuseReflectance(eta);
            return Kd_ * (1.0 - Fo) * (1.0 - Fi) * invEta * invEta * INV_PI / (1.0 - Fdr);
        }
    }

    double pdf(const Vector3d& wo,
               const Vector3d& wi) const override {
        if (!vect::sameHemisphere(wo, wi)) return 0.0;
        
        const Vector3d wh = vect::normalize(wi + wo);
        const double F = FrDielectric(vect::dot(wo, wh), etaA_, etaB_);
        const double specPDF = F * distrib_->pdf(wo, wh) / (4.0 * vect::absDot(wo, wh));
        const double diffPDF = (1.0 - F) * std::abs(vect::cosTheta((wi))) * INV_PI;
        return specPDF + diffPDF;
    }
    
private:
    Spectrum Kd_, Ks_;
    double etaA_, etaB_;
    MicrofacetDistribution *distrib_;
    double specularSamplingWeight_;
};

}  // anonymous namespace

RoughPlastic::RoughPlastic(const std::shared_ptr<Texture<Spectrum>>& Kd,
                           const std::shared_ptr<Texture<Spectrum>>& Ks,
                           const std::shared_ptr<Texture<double>> &index,
                           const std::shared_ptr<Texture<double>>& roughness,
                           const std::string &distribution,
                           const std::shared_ptr<Texture<double>>& bumpMap,
                           bool remapRoughness)
    : SurfaceMaterial{}
    , Kd_{ Kd }
    , Ks_{ Ks }
    , roughness_{ roughness }
    , index_{ index }
    , distribution_{ distribution }
    , bumpMap_{ bumpMap }
    , remapRoughness_{ remapRoughness } {
}

RoughPlastic::RoughPlastic(RenderParams &params)
    : RoughPlastic{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("diffuseReflectance")),
                   std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularReflectance")),
                   std::static_pointer_cast<Texture<double>>(params.getTexture("intIOR", 1.3333)),
                   std::static_pointer_cast<Texture<double>>(params.getTexture("alpha", 0.1)),
                   params.getString("distribution", "beckmann", true),
                   std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void RoughPlastic::setScatterFuncs(SurfaceInteraction* isect,
                                      MemoryArena& arena) const {
    if (bumpMap_) bump(isect, bumpMap_);

    const Spectrum kd = Kd_->evaluate(*isect);
    const Spectrum ks = Ks_->evaluate(*isect);
    const double eta = index_->evaluate(*isect);
    double rough = roughness_->evaluate(*isect);
    
    isect->setBSDF(arena.allocate<BSDF>(*isect));
    if (!ks.isBlack()) {
        if (remapRoughness_) {
            if (distribution_ == "backmann") {
                rough = BeckmannDistribution::roughnessToAlpha(rough);
            } else if (distribution_ == "ggx") {
                rough = TrowbridgeReitzDistribution::roughnessToAlpha(rough);
            }
        }
        
        MicrofacetDistribution *distrib = nullptr;
        if (distribution_ == "beckmann") {
            distrib = arena.allocate<BeckmannDistribution>(rough, rough);
        } else if (distribution_ == "ggx") {
            distrib = arena.allocate<TrowbridgeReitzDistribution>(rough, rough);
        } else {
            FatalError("Unknown microfacet distribution type: %s", distribution_.c_str());
        }

        isect->bsdf()->add(arena.allocate<RoughPlasticBRDF>(kd, ks, distrib, 1.0, eta));
    } else {
        isect->bsdf()->add(arena.allocate<LambertianReflection>(kd));
    }
}

}  // namespace spica
