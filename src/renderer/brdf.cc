#define SPICA_BRDF_EXPORT
#include "brdf.h"

#include "../utils/sampler.h"

#include "bsdf.h"
#include "renderer_helper.h"
#include "renderer_constants.h"

namespace spica {

    // --------------------------------------------------
    // Lambertian BRDF
    // --------------------------------------------------

    LambertianBRDF::LambertianBRDF(const Color& reflectance, const Color& emittance)
        : _reflectance(reflectance)
        , _emittance(emittance)
    {
    }

    Color LambertianBRDF::reflectance() const {
        return _reflectance;
    }

    Color LambertianBRDF::emittance() const {
        return _emittance;
    }

    void LambertianBRDF::sample(const Vector3D& in, const Vector3D& normal, double rand1, double rand2, Vector3D* out, double* pdf) const {
        const Vector3D orieintingNormal = in.dot(normal) < 0.0 ? normal : -normal;

        (*pdf) = 1.0;
        sampler::onHemisphere(orieintingNormal, out, rand1, rand2);
    }

    BSDF LambertianBRDF::factory(const Color& reflectance, const Color& emittance) {
        return std::move(BSDF(new LambertianBRDF(reflectance, emittance), BSDF_TYPE_LAMBERTIAN_BRDF));
    }

    BSDFBase* LambertianBRDF::clone() const {
        return new LambertianBRDF(_reflectance, _emittance);
    }

    // --------------------------------------------------
    // Specular BRDF
    // --------------------------------------------------

    SpecularBRDF::SpecularBRDF(const Color& reflectance, const Color& emittance)
        : _reflectance(reflectance)
        , _emittance(emittance)
    {
    }

    Color SpecularBRDF::reflectance() const {
        return _reflectance;
    }

    Color SpecularBRDF::emittance() const {
        return _emittance;
    }

    void SpecularBRDF::sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out, double* pdf) const {
        const Vector3D orieintingNormal = in.dot(normal) < 0.0 ? normal : -normal;

        (*pdf) = 1.0;
        (*out) = Vector3D::reflect(in, orieintingNormal);
    }

    BSDF SpecularBRDF::factory(const Color& reflectance, const Color& emittance) {
        return std::move(BSDF(new SpecularBRDF(reflectance, emittance), BSDF_TYPE_SPECULAR_BRDF));
    }

    BSDFBase* SpecularBRDF::clone() const {
        return new SpecularBRDF(_reflectance, _emittance);
    }

    // --------------------------------------------------
    // Phong BRDF
    // --------------------------------------------------

    PhongBRDF::PhongBRDF(const Color& reflectance, const Color& emittance, const double n)
        : _reflectance(reflectance)
        , _emittance(emittance)
        , _coeff(n)
    {
    }

    Color PhongBRDF::reflectance() const {
        return _reflectance;
    }

    Color PhongBRDF::emittance() const {
        return _emittance;
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

    BSDF PhongBRDF::factory(const Color& reflectance, const Color& emittance, const double n) {
        return std::move(BSDF(new PhongBRDF(reflectance, emittance, n), BSDF_TYPE_PHONG_BRDF));
    }

    BSDFBase* PhongBRDF::clone() const {
        return new PhongBRDF(_reflectance, _emittance, _coeff);
    }

    // --------------------------------------------------
    // Refractive BSDF
    // --------------------------------------------------
    
    RefractiveBSDF::RefractiveBSDF(const Color& reflectance, const Color& emittance)
        : _reflectance(reflectance)
        , _emittance(emittance)
    {
    }

    Color RefractiveBSDF::reflectance() const {
        return _reflectance;
    }

    Color RefractiveBSDF::emittance() const {
        return _emittance;
    }

    void RefractiveBSDF::sample(const Vector3D& in, const Vector3D& normal, double rand1, double rand2, Vector3D* out, double* pdf) const {
        const Vector3D orientingNormal = in.dot(normal) < 0.0 ? normal : -normal;
        const bool into = normal.dot(orientingNormal) > 0.0;

        Vector3D reflectdir, transmitdir;
        double fresnelRe, fresnelTr;
        const bool isTotalReflectance = helper::checkTotalReflection(into, in, normal, orientingNormal, &reflectdir, &transmitdir, &fresnelRe, &fresnelTr);
    
        if (isTotalReflectance) {
            *out = reflectdir;
            *pdf = 1.0;
        } else {
            const double probability = 0.25 + 0.5 * kReflectProbability;
            if (rand1 < probability) {
                *pdf = probability / fresnelRe;
                *out = reflectdir;
            } else {
                *pdf = (1.0 - probability) / fresnelTr;
                *out = transmitdir;
            }
        }
    }

    BSDF RefractiveBSDF::factory(const Color& reflectance, const Color& emittance) {
        return std::move(BSDF(new RefractiveBSDF(reflectance, emittance), BSDF_TYPE_REFRACTION));
    }

    BSDFBase* RefractiveBSDF::clone() const {
        return new RefractiveBSDF(_reflectance, _emittance);
    }



}
