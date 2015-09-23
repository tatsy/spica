#include "area_light.h"

namespace spica {

    // --------------------------------------------------
    // Diffuse area light
    // --------------------------------------------------

    DiffuseAreaLight::DiffuseAreaLight(const Triangle& tri, const Color& emittance)
        : _emittance(emittance)
        , _triangle(tri) {
    }

    DiffuseAreaLight::~DiffuseAreaLight() {
    }

    Color DiffuseAreaLight::L(const Vector3D& pos, const Vector3D& normal, const Vector3D& w) const {
        return normal.dot(w) > 0.0f ? _emittance : Color(0.0, 0.0, 0.0);
    }

    // --------------------------------------------------
    // Infinite area light
    // --------------------------------------------------

    InfiniteAreaLight::InfiniteAreaLight(const Color& lightness, const Image& envmap)
        : _lightness(lightness)
        , _envmap(envmap) {
        computeSamplePdf();
    }

    InfiniteAreaLight::~InfiniteAreaLight() {
    }
    
    Color InfiniteAreaLight::L(const Vector3D& pos, const Vector3D& normal, const Vector3D& w) const {
        return Color::BLACK;
    }

    void InfiniteAreaLight::computeSamplePdf() {
        const int width  = _envmap.width();
        const int height = _envmap.height();
        double filter = 1.0 / std::max(width, height);
        for (int v = 0; v < height; v++) {
            double vp = static_cast<double>(v) / height;
            double sint = PI * (v + 0.5) / (double)height;
            for (int u = 0; u < width; u++) {
                double up = static_cast<double>(u) / width;
                
            }
        }
    }

}  // namespace spica
