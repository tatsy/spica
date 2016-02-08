#define SPICA_API_EXPORT
#include "brdf.h"

#include "../core/sampler.h"

#include "bsdf.h"
#include "../math/vect_math.h"
#include "../renderer/renderer_helper.h"
#include "../renderer/renderer_constants.h"

namespace spica {

    // --------------------------------------------------
    // Lambertian BRDF
    // --------------------------------------------------

    LambertianBRDF::LambertianBRDF(const Spectrum& reflectance)
        : AbstractBSDF{}
        , reflectance_{ reflectance } {
    }

    const Spectrum& LambertianBRDF::reflectance() const {
        return reflectance_;
    }

    void LambertianBRDF::sample(const Vector3D& in, const Normal& normal, double rand1, double rand2, Vector3D* out, double* pdf) const {
        const Normal orieintingNormal = vect::dot(in, normal) < 0.0 ? normal : -normal;

        (*pdf) = 1.0;
        sampler::onHemisphere(orieintingNormal, out, rand1, rand2);
    }

    double LambertianBRDF::pdf(const Vector3D& in, const Normal& normal, const Vector3D& out) const {
        return vect::dot(normal, out) * INV_PI;
    }

    BSDF LambertianBRDF::factory(const Spectrum& reflectance) {
        return std::move(BSDF(new LambertianBRDF(reflectance), BsdfType::Lambertian));
    }

    AbstractBSDF* LambertianBRDF::clone() const {
        return new LambertianBRDF(reflectance_);
    }

    // --------------------------------------------------
    // Specular BRDF
    // --------------------------------------------------

    SpecularBRDF::SpecularBRDF(const Spectrum& reflectance)
        : AbstractBSDF{}
        , reflectance_{ reflectance } {
    }

    const Spectrum& SpecularBRDF::reflectance() const {
        return reflectance_;
    }

    void SpecularBRDF::sample(const Vector3D& in, const Normal& normal, const double rand1, const double rand2, Vector3D* out, double* pdf) const {
        const Normal orieintingNormal = vect::dot(in, normal) < 0.0 ? normal : -normal;

        (*pdf) = 1.0;
        (*out) = vect::reflect(in, orieintingNormal);
    }

    double SpecularBRDF::pdf(const Vector3D& in, const Normal& normal, const Vector3D& out) const {
        Vector3D refdir = vect::reflect(in, normal);
        if (Vector3D::dot(refdir, out) > (1.0 - EPS) ) {
            return 1.0;
        }
        return 0.0;
    }

    BSDF SpecularBRDF::factory(const Spectrum& reflectance) {
        return std::move(BSDF(new SpecularBRDF(reflectance), BsdfType::Specular));
    }

    AbstractBSDF* SpecularBRDF::clone() const {
        return new SpecularBRDF(reflectance_);
    }

    // --------------------------------------------------
    // Phong BRDF
    // --------------------------------------------------

    PhongBRDF::PhongBRDF(const Spectrum& reflectance, const double n)
        : AbstractBSDF{}
        , reflectance_{ reflectance }
        , coeff_{ n } {
    }

    const Spectrum& PhongBRDF::reflectance() const {
        return reflectance_;
    }

    void PhongBRDF::sample(const Vector3D& in, const Normal& normal, const double rand1, const double rand2, Vector3D* out, double* pdf) const {
        const Normal orieintingNormal = vect::dot(in, normal) < 0.0 ? normal : -normal;

        const Vector3D refDir = vect::reflect(in, orieintingNormal);

        Vector3D u, v, w;
        w = refDir;
        helper::calcLocalCoords(w, &u, &v);

        const double theta = acos(pow(rand1, 1.0 / (coeff_ + 1.0)));
        const double phi   = 2.0 * PI * rand2;

        (*pdf) = 1.0;
        (*out) = (u * sin(theta) * cos(phi) + w * cos(theta) + v * sin(theta) * sin(phi)).normalized();
    }

    double PhongBRDF::pdf(const Vector3D& in, const Normal& normal, const Vector3D& out) const {
        Vector3D refdir = vect::reflect(in, normal);
        double cosine = std::max(0.0, Vector3D::dot(refdir, out));
        return (coeff_ + 1.0) / (2.0 * PI) * pow(cosine, coeff_);
    }

    BSDF PhongBRDF::factory(const Spectrum& reflectance, const double n) {
        return std::move(BSDF(new PhongBRDF(reflectance, n), BsdfType::PhongBrdf));
    }

    AbstractBSDF* PhongBRDF::clone() const {
        return new PhongBRDF(reflectance_, coeff_);
    }

    // --------------------------------------------------
    // Refractive BSDF
    // --------------------------------------------------
    
    RefractiveBSDF::RefractiveBSDF(const Spectrum& reflectance)
        : AbstractBSDF{}
        , reflectance_{ reflectance } {
    }

    const Spectrum& RefractiveBSDF::reflectance() const {
        return reflectance_;
    }

    void RefractiveBSDF::sample(const Vector3D& in, const Normal& normal, double rand1, double rand2, Vector3D* out, double* pdf) const {
        const Normal orientN = vect::dot(in, normal) < 0.0 ? normal : -normal;
        const bool into = vect::dot(normal, orientN) > 0.0;

        Vector3D reflectdir, transmitdir;
        double fresnelRe, fresnelTr;
        const bool isTotalReflectance = 
            helper::checkTotalReflection(into, in,
                                         normal, orientN,
                                         &reflectdir, &transmitdir,
                                         &fresnelRe, &fresnelTr);
    
        if (isTotalReflectance) {
            (*out) = reflectdir;
            (*pdf) = 1.0;
        } else {
            const double probability = 0.25 + 0.5 * fresnelRe;
            if (rand1 < probability) {
                (*pdf) = probability / fresnelRe;
                (*out) = reflectdir;
            } else {
                (*pdf) = (1.0 - probability) / fresnelTr;
                (*out) = transmitdir;
            }
        }
    }

    double RefractiveBSDF::pdf(const Vector3D& in, const Normal& normal, const Vector3D& out) const {
        const Normal orientN = vect::dot(in, normal) < 0.0 ? normal : -normal;
        const bool into = vect::dot(normal, orientN) > 0.0;

        Vector3D reflectdir, transmitdir;
        double fresnelRe, fresnelTr;
        const bool isTotalReflectance = 
            helper::checkTotalReflection(into, in,
                                         normal, orientN,
                                         &reflectdir, &transmitdir,
                                         &fresnelRe, &fresnelTr);
    
        if (isTotalReflectance) {
            if (Vector3D::dot(reflectdir, out) > (1.0 - EPS)) { 
                return 1.0;
            }
            return 0.0;
        } else {
            if (Vector3D::dot(reflectdir, out) > (1.0 - EPS)) {
                return 1.0 / fresnelRe;
            } else if (Vector3D::dot(transmitdir, out) > (1.0 - EPS)) {
                return 1.0 / fresnelTr;            
            }
            return 0.0;
        }                
    }

    BSDF RefractiveBSDF::factory(const Spectrum& reflectance) {
        return std::move(BSDF(new RefractiveBSDF(reflectance), BsdfType::Refractive));
    }

    AbstractBSDF* RefractiveBSDF::clone() const {
        return new RefractiveBSDF(reflectance_);
    }



}
