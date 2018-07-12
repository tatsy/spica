#define SPICA_API_EXPORT
#include "plastic.h"

#include "core/memory.h"
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

class PlasticBRDF : public BxDF {
public:
    PlasticBRDF(const Spectrum &Kd, const Spectrum &Ks, double etaA, double etaB)
        : BxDF{BxDFType::Reflection | BxDFType::Specular | BxDFType::Diffuse}
        , Kd_{Kd}
        , Ks_{Ks}
        , etaA_{etaA}
        , etaB_{etaB} {
        specularSamplingWeight_ = Ks_.luminance() / (Kd_.luminance() + Ks_.luminance());
    }
    
    
    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override {
        const double Fo = FrDielectric(vect::cosTheta(wo), etaA_, etaB_);

        if (vect::dot(wi, Vector3d(-wi.x(), -wi.y(), wi.z())) > 1.0 - EPS) {
            // Specular reflection
            return Fo * Ks_ / std::abs(vect::cosTheta(wi));
        } else {
            // Diffuse reflection
            const double eta = etaB_ / etaA_;
            const double invEta = 1.0 / eta;
            const double Fi = FrDielectric(vect::cosTheta(wi), etaA_, etaB_);
            const double Fdr = FrDiffuseReflectance(eta);
            return (1.0 - Fo) * (1.0 - Fi) * invEta * invEta * Kd_ * INV_PI / (1.0 - Fdr);
        }
    }

    Spectrum sample(const Vector3d& wo, Vector3d* wi,
                    const Point2d& rands, double* pdf,
                    BxDFType* sampledType) const override {
        const double Fo = FrDielectric(vect::cosTheta(wo), etaA_, etaB_);
        const double probSpecular = (Fo * specularSamplingWeight_) /
                                    (Fo * specularSamplingWeight_ + (1.0 - Fo) * (1.0 - specularSamplingWeight_));
        if (random.get1D() < probSpecular) {
            // Specular reflection
            *wi = Vector3d(-wo.x(), -wo.y(), wo.z());
            *pdf = probSpecular;
            return Ks_ * Fo / std::abs(vect::cosTheta(*wi));
        } else {
            // Diffuse reflection
            *wi = sampleCosineHemisphere(rands);
            const double eta = etaB_ / etaA_;
            const double invEta = 1.0 / eta;
            const double Fi = FrDielectric(vect::cosTheta(*wi), etaA_, etaB_);
            const double Fdr = FrDiffuseReflectance(eta);
            *pdf = (1.0 - probSpecular) * std::abs(vect::cosTheta(*wi)) * INV_PI;
            return (1.0 - Fo) * (1.0 - Fi) * invEta * invEta * Kd_ * INV_PI / (1.0 - Fdr);
        }
    }

    double pdf(const Vector3d& wo, const Vector3d& wi) const override {
        const double Fo = FrDielectric(vect::cosTheta(wo), etaA_, etaB_);
        const double probSpecular = (Fo * specularSamplingWeight_) /
                                    (Fo * specularSamplingWeight_ + (1.0 - Fo) * (1.0 - specularSamplingWeight_));

        if (vect::dot(wi, Vector3d(-wi.x(), -wi.y(), wi.z())) > 1.0 - EPS) {
            // Specular reflection
            return probSpecular;
        } else {
            // Diffuse reflection
            return (1.0 - probSpecular) * std::abs(vect::cosTheta(wi)) * INV_PI;
        }
    }
    
private:
    Spectrum Kd_, Ks_;
    double etaA_, etaB_;
    double specularSamplingWeight_ = 1.0;
};
    
}  // anonymous namespace

Plastic::Plastic(const std::shared_ptr<Texture<Spectrum>>& Kd,
                 const std::shared_ptr<Texture<Spectrum>>& Ks,
                 const std::shared_ptr<Texture<double>> &eta,
                 const std::shared_ptr<Texture<double>>& bumpMap)
    : SurfaceMaterial{}
    , Kd_{ Kd }
    , Ks_{ Ks }
    , eta_{ eta }
    , bumpMap_{ bumpMap } {
}

Plastic::Plastic(RenderParams &params)
    : Plastic{std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("diffuseReflectance")),
              std::static_pointer_cast<Texture<Spectrum>>(params.getTexture("specularReflectance")),
              std::static_pointer_cast<Texture<double>>(params.getTexture("intIOR")),
              std::static_pointer_cast<Texture<double>>(params.getTexture("bumpMap"))} {
}

void Plastic::setScatterFuncs(SurfaceInteraction* isect,
                              MemoryArena& arena) const {
    if (bumpMap_) bump(isect, bumpMap_);

    const Spectrum kd = Kd_->evaluate(*isect);
    const Spectrum ks = Ks_->evaluate(*isect);
    const double eta = eta_->evaluate(*isect);

    isect->setBSDF(arena.allocate<BSDF>(*isect));
    isect->bsdf()->add(arena.allocate<PlasticBRDF>(kd, ks, 1.0, eta));
}

}  // namespace spica
