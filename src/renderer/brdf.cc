#define SPICA_BRDF_EXPORT
#include "brdf.h"

#include "../utils/sampler.h"

#include "bsdf.h"

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

    void LambertianBRDF::sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const {
        sampler::onHemisphere(normal, out, rand1, rand2);
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

    void SpecularBRDF::sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const {
        (*out) = Vector3D::reflect(in, normal);
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

    void PhongBRDF::sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const {
        Vector3D refDir = Vector3D::reflect(in, normal);

        Vector3D u, v, w;
        w = refDir;
        if (std::abs(w.x()) > EPS) {
            u = Vector3D(0.0, 1.0, 0.0).cross(w).normalized();
        }
        else {
            u = Vector3D(1.0, 0.0, 0.0).cross(w).normalized();
        }
        v = w.cross(u);

        double theta = acos(pow(rand1, 1.0 / (_coeff + 1.0)));
        double phi = 2.0 * PI * rand2;

        (*out) = u * sin(theta) * cos(phi) + w * cos(theta) + v * sin(theta) * sin(phi);
    }

    BSDF PhongBRDF::factory(const Color& reflectance, const Color& emittance, const double n) {
        return std::move(BSDF(new PhongBRDF(reflectance, emittance, n), BSDF_TYPE_PHONG_BRDF));
    }

    BSDFBase* PhongBRDF::clone() const {
        return new PhongBRDF(_reflectance, _emittance, _coeff);
    }

}
