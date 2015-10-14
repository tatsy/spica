#define SPICA_API_EXPORT
#include "brdf.h"

#include "../core/sampler.h"

#include "bsdf.h"
#include "../renderer/renderer_helper.h"
#include "../renderer/renderer_constants.h"

namespace spica {

    // --------------------------------------------------
    // Lambertian BRDF
    // --------------------------------------------------

    LambertianBRDF::LambertianBRDF(const Color& reflectance)
        : _reflectance(reflectance)
    {
    }

    const Color& LambertianBRDF::reflectance() const {
        return _reflectance;
    }

    void LambertianBRDF::sample(const Vector3D& in, const Vector3D& normal, double rand1, double rand2, Vector3D* out, double* pdf) const {
        const Vector3D orieintingNormal = in.dot(normal) < 0.0 ? normal : -normal;

        (*pdf) = 1.0;
        sampler::onHemisphere(orieintingNormal, out, rand1, rand2);
    }

    BSDF LambertianBRDF::factory(const Color& reflectance) {
        return std::move(BSDF(new LambertianBRDF(reflectance), BsdfType::Lambertian));
    }

    BSDFBase* LambertianBRDF::clone() const {
        return new LambertianBRDF(_reflectance);
    }

    // --------------------------------------------------
    // Specular BRDF
    // --------------------------------------------------

    SpecularBRDF::SpecularBRDF(const Color& reflectance)
        : _reflectance(reflectance)
    {
    }

    const Color& SpecularBRDF::reflectance() const {
        return _reflectance;
    }

    void SpecularBRDF::sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out, double* pdf) const {
        const Vector3D orieintingNormal = in.dot(normal) < 0.0 ? normal : -normal;

        (*pdf) = 1.0;
        (*out) = Vector3D::reflect(in, orieintingNormal);
    }

    BSDF SpecularBRDF::factory(const Color& reflectance) {
        return std::move(BSDF(new SpecularBRDF(reflectance), BsdfType::Specular));
    }

    BSDFBase* SpecularBRDF::clone() const {
        return new SpecularBRDF(_reflectance);
    }

    // --------------------------------------------------
    // Phong BRDF
    // --------------------------------------------------

    PhongBRDF::PhongBRDF(const Color& reflectance, const double n)
        : _reflectance(reflectance)
        , _coeff(n)
    {
    }

    const Color& PhongBRDF::reflectance() const {
        return _reflectance;
    }

    void PhongBRDF::sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out, double* pdf) const {
        const Vector3D orieintingNormal = in.dot(normal) < 0.0 ? normal : -normal;

        const Vector3D refDir = Vector3D::reflect(in, orieintingNormal);

        Vector3D u, v, w;
        w = refDir;
        helper::calcLocalCoords(w, &u, &v);

        const double theta = acos(pow(rand1, 1.0 / (_coeff + 1.0)));
        const double phi   = 2.0 * PI * rand2;

        (*pdf) = 1.0;
        (*out) = (u * sin(theta) * cos(phi) + w * cos(theta) + v * sin(theta) * sin(phi)).normalized();
    }

    BSDF PhongBRDF::factory(const Color& reflectance, const double n) {
        return std::move(BSDF(new PhongBRDF(reflectance, n), BsdfType::PhongBrdf));
    }

    BSDFBase* PhongBRDF::clone() const {
        return new PhongBRDF(_reflectance, _coeff);
    }

    // --------------------------------------------------
    // Refractive BSDF
    // --------------------------------------------------
    
    RefractiveBSDF::RefractiveBSDF(const Color& reflectance)
        : _reflectance(reflectance)
    {
    }

    const Color& RefractiveBSDF::reflectance() const {
        return _reflectance;
    }

    void RefractiveBSDF::sample(const Vector3D& in, const Vector3D& normal, double rand1, double rand2, Vector3D* out, double* pdf) const {
        const Vector3D orientingNormal = in.dot(normal) < 0.0 ? normal : -normal;
        const bool into = normal.dot(orientingNormal) > 0.0;

        Vector3D reflectdir, transmitdir;
        double fresnelRe, fresnelTr;
        const bool isTotalReflectance = helper::checkTotalReflection(into, in, normal, orientingNormal, &reflectdir, &transmitdir, &fresnelRe, &fresnelTr);
    
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

    BSDF RefractiveBSDF::factory(const Color& reflectance) {
        return std::move(BSDF(new RefractiveBSDF(reflectance), BsdfType::Refractive));
    }

    BSDFBase* RefractiveBSDF::clone() const {
        return new RefractiveBSDF(_reflectance);
    }



}
