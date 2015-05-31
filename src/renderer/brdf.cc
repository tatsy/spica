#define SPICA_BRDF_EXPORT
#include "brdf.h"

#include "../utils/sampler.h"

namespace spica {

    // --------------------------------------------------
    // Lambertian BRDF
    // --------------------------------------------------

    LambertianBRDF::LambertianBRDF(const Color& reflectance)
        : _reflectance(reflectance)
    {
    }

    Color LambertianBRDF::reflectance() const {
        return _reflectance;
    }

    void LambertianBRDF::sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const {
        sampler::onHemisphere(normal, out, rand1, rand2);
    }

    // --------------------------------------------------
    // Specular BRDF
    // --------------------------------------------------

    SpecularBRDF::SpecularBRDF(const Color& reflectance)
        : _reflectance(reflectance)
    {
    }

    Color SpecularBRDF::reflectance() const {
        return _reflectance;
    }

    void SpecularBRDF::sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const {
        (*out) = Vector3::reflect(in, normal);
    }

    // --------------------------------------------------
    // Phong BRDF
    // --------------------------------------------------

    PhongBRDF::PhongBRDF(const Color& reflectance, const double n)
        : _reflectance(reflectance)
        , _coeffN(n)
    {
    }

    Color PhongBRDF::reflectance() const {
        return _reflectance;
    }

    void PhongBRDF::sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const {
        Vector3 refDir = Vector3::reflect(in, normal);

        Vector3 u, v, w;
        w = normal;
        if (abs(w.x()) > EPS) {
            u = Vector3(0.0, 1.0, 0.0).cross(w).normalized();
        }
        else {
            u = Vector3(1.0, 0.0, 0.0).cross(w).normalized();
        }
        v = w.cross(u);

        double theta = acos(pow(rand1, 1.0 / (_coeffN + 1.0)));
        double phi = 2.0 * PI * rand2;

        (*out) = u * sin(theta) * cos(phi) + w * cos(theta) + v * sin(theta) * sin(phi);
    }

}
