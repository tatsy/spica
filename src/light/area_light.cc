#define SPICA_API_EXPORT
#include "area_light.h"

#include "../core/sampler.h"

namespace spica {

    AreaLight::AreaLight()
        : ILight{LightType::AreaLight}
        , _emittance{}
        , _triangles{}
        , _samplePdf{}
        , _totalArea{0.0} {
    }

    AreaLight::AreaLight(const Trimesh& tris, const Color& emittance)
        : ILight{LightType::AreaLight}
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
        ILight::operator=(l);
        this->_emittance = l._emittance;
        this->_triangles = l._triangles;
        this->_samplePdf = l._samplePdf;
        this->_totalArea = l._totalArea;
        return *this;
    }

    AreaLight& AreaLight::operator=(AreaLight&& l) {
        ILight::operator=(std::move(l));
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

    LightSample AreaLight::sample(double r1, double r2, double r3) const {
        Assertion(!_samplePdf.empty(), "Light PDFs are not computed!!");

        const int id = std::lower_bound(_samplePdf.begin(), _samplePdf.end(), r1) - _samplePdf.begin();
        const Triangle& tri = _triangles[id];

        Vector3D p, n;
        sampler::onTriangle(tri, &p, &n, r2, r3);

        return LightSample(p, n, _emittance, _totalArea);
    }

    Color AreaLight::directLight(const Vector3D& dir) const {
        return _emittance;
    }

    double AreaLight::area() const {
        return _totalArea;
    }

    ILight* AreaLight::clone() const {
        return new AreaLight(*this);
    }

}  // namespace spica
