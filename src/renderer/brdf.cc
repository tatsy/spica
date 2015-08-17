#define SPICA_BRDF_EXPORT
#include "brdf.h"

#include "../utils/sampler.h"

namespace spica {

    // --------------------------------------------------
    // BRDF
    // --------------------------------------------------
    BRDF::BRDF()
        : _numCopies(NULL)
        , _ptr(NULL)
    {
    }

    BRDF::BRDF(const BRDF& brdf)
        : _numCopies(NULL)
        , _ptr(NULL)
    {
        this->operator=(brdf);
    }

    BRDF::BRDF(const BRDFBase* ptr)
        : _numCopies(new int(0))
        , _ptr(ptr)
    {
    }

    BRDF::~BRDF()
    {
        release();
    }

    BRDF& BRDF::operator=(const BRDF& brdf) {
        release();
        _numCopies = brdf._numCopies;
        _ptr = brdf._ptr;
        if (_numCopies != NULL) {
            (*_numCopies)++;
        }
        return *this;
    }

    Color BRDF::reflectance() const {
        return _ptr->reflectance();
    }

    void BRDF::sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const {
        _ptr->sample(in, normal, rand1, rand2, out);
    }

    void BRDF::release() {
        if (_numCopies != NULL) {
            if ((*_numCopies) == 0) {
                delete _numCopies;
                delete _ptr;
                _numCopies = NULL;
                _ptr = NULL;
            } else {
                (*_numCopies) -= 1;
            }
        }
    }

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

    void LambertianBRDF::sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const {
        sampler::onHemisphere(normal, out, rand1, rand2);
    }

    BRDF LambertianBRDF::factory(const Color& reflectance) {
        return BRDF(new LambertianBRDF(reflectance));
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

    void SpecularBRDF::sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const {
        (*out) = Vector3D::reflect(in, normal);
    }

    BRDF SpecularBRDF::factory(const Color& reflectance) {
        return BRDF(new SpecularBRDF(reflectance));
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

        double theta = acos(pow(rand1, 1.0 / (_coeffN + 1.0)));
        double phi = 2.0 * PI * rand2;

        (*out) = u * sin(theta) * cos(phi) + w * cos(theta) + v * sin(theta) * sin(phi);
    }

    BRDF PhongBRDF::factory(const Color& reflectance, const double n) {
        return BRDF(new PhongBRDF(reflectance, n));
    }

}
