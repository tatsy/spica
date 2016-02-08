#define SPICA_API_EXPORT
#include "area_light.h"

#include "../core/sampler.h"
#include "../renderer/photon_map.h"

namespace spica {

    AreaLight::AreaLight()
        : Light{LightType::Area}
        , _emittance{}
        , _triangles{}
        , _samplePdf{}
        , _totalArea{0.0} {
    }

    AreaLight::AreaLight(const Trimesh& tris, const Color& emittance)
        : Light{LightType::Area}
        , _emittance{emittance}
        , _triangles{}
        , _samplePdf{}
        , _totalArea{} {
        for (int i = 0; i < tris.numFaces(); i++) {
            _triangles.push_back(tris.getTriangle(i));
        }
        calcSamplePdf();
    }

    AreaLight::~AreaLight() {
    }

    AreaLight::AreaLight(const AreaLight& l)
        : AreaLight{} {
        this->operator=(l);
    }

    AreaLight::AreaLight(AreaLight&& l)
        : AreaLight{} {
        this->operator=(std::move(l));
    }

    AreaLight& AreaLight::operator=(const AreaLight& l) {
        Light::operator=(l);
        this->_emittance = l._emittance;
        this->_triangles = l._triangles;
        this->_samplePdf = l._samplePdf;
        this->_totalArea = l._totalArea;
        return *this;
    }

    AreaLight& AreaLight::operator=(AreaLight&& l) {
        Light::operator=(std::move(l));
        this->_emittance = l._emittance;
        this->_triangles = std::move(l._triangles);
        this->_samplePdf = std::move(l._samplePdf);
        this->_totalArea = l._totalArea;
        return *this;
    }

    void AreaLight::calcSamplePdf() {
        if (_triangles.empty()) return;
        _samplePdf.resize(_triangles.size());

        _totalArea = 0.0;
        for (int i = 0; i < _triangles.size(); i++) {
            const double A = _triangles[i].area();
            _samplePdf[i] = A;
            _totalArea += A;
        }

        _samplePdf[0] /= _totalArea;
        for (int i = 1; i < _triangles.size(); i++) {
            _samplePdf[i] = _samplePdf[i - 1] + _samplePdf[i] / _totalArea;
        }
    }

    LightSample AreaLight::sample(const Vector3D& v, Stack<double>& rands) const {
        // Randomly choose point on the triangle
        Vector3D p, n;
        sampleOnLight(&p, &n, rands);

        // Compute PDF
        Vector3D dir = (p - v).normalized();
        double dist2 = (p - v).squaredNorm();
        const double dot1 = Vector3D::dot(n, -dir);
        double pdf = 0.0;
        if (dot1 > EPS) {
            pdf = 1.0 / (INV_PI * (dot1 / dist2) * area());
        }
        return LightSample(p, n, -dir, _emittance, pdf);
    }

    Photon AreaLight::samplePhoton(Stack<double>& rands) const {
        // Randomly choose point on the triangle
        Vector3D p, n;
        sampleOnLight(&p, &n, rands);

        // Random direction sample (Lambertian surface)
        Vector3D dir;
        sampler::onHemisphere(n, &dir, rands.pop(), rands.pop());

        Color flux = PI * _emittance * area();
        return Photon(p, flux, dir, n);
    }

    void AreaLight::sampleOnLight(Vector3D* pos, Vector3D* nrm, Stack<double>& rands) const {
        Assertion(!_samplePdf.empty(), "Light PDFs are not computed!!");

        // Randomly choose triangle
        const int id = std::lower_bound(_samplePdf.begin(), _samplePdf.end(), rands.pop()) - _samplePdf.begin();
        const Triangle& tri = _triangles[id];

        // Randomly choose point on the triangle
        sampler::onTriangle(tri, pos, nrm, rands.pop(), rands.pop());            
    }

    Color AreaLight::directLight(const Vector3D& dir) const {
        return _emittance;
    }

    Color AreaLight::globalLight(const Vector3D& dir) const {
        return Color::BLACK;
    }

    double AreaLight::area() const {
        return _totalArea;
    }

    Light* AreaLight::clone() const {
        return new AreaLight(*this);
    }

}  // namespace spica
