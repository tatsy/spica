#define SPICA_API_EXPORT
#include "envmap.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "../core/common.h"
#include "../core/point2d.h"
#include "../core/normal3d.h"
#include "../core/sampling.h"
#include "../shape/visibility_tester.h"

namespace spica {

Envmap::Envmap(const Image& texmap, const Transform& lightToWorld,
                const Spectrum& L, int numSamples)
    : Light{ LightType::Envmap, lightToWorld, numSamples }
    , texmap_{ texmap, ImageWrap::Repeat }
    , worldCenter_{}
    , worldRadius_{}
    , distrib_{} {
    const int width  = texmap.width();
    const int height = texmap.height();
    std::vector<double> gray(width * height);
    for (int y = 0; y < height; y++) {
        double v = static_cast<double>(y) / height;
        double sint = sin(PI * (v + 0.5)) / height;
        for (int x = 0; x < width; x++) {
            double u = static_cast<double>(x) / width;
            // TODO: Better to use mipmap, i.e. interpolated gray value
            gray[y * width + x] = texmap(x, y).luminance();
            gray[y * width + x] *= sint;
        }
    }
    distrib_ = Distribution2D(gray, width, height);
}

Envmap::~Envmap() {
}

Spectrum Envmap::sampleLi(const Interaction& pObj, const Point2d& rands,
                            Vector3d* dir, double* pdf, VisibilityTester* vis) const {
    double mapPdf;
    Point2d uv = distrib_.sample(rands, &mapPdf);
    if (mapPdf == 0.0) return Spectrum();

    const double theta = uv[1] * PI;
    const double phi   = uv[0] * (2.0 * PI);
    const double cosTheta = cos(theta);
    const double sinTheta = sin(theta);
    const double cosPhi   = cos(phi);
    const double sinPhi   = sin(phi);
    *dir = lightToWorld_.apply(Vector3d(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta));
    *pdf = sinTheta == 0.0 ? 0.0 : mapPdf / (2.0 * PI * PI * sinTheta);

    Interaction pLight(pObj.pos() + (*dir) * (2.0 * worldRadius_));
    *vis = VisibilityTester(pObj, pLight);
        
    return Spectrum(texmap_.lookup(uv));
}

double Envmap::pdfLi(const Interaction&, const Vector3d& d) const {
    Vector3d dir = worldToLight_.apply(d);
    double theta = vect::sphericalTheta(dir);
    double phi   = vect::sphericalPhi(dir);
    double sinTheta = sin(theta);
    if (sinTheta == 0.0) return 0.0;
    return distrib_.pdf(Point2d(phi * (0.5 * INV_PI), theta * INV_PI)) / (2.0 * PI * PI * sinTheta);
}

Spectrum Envmap::Le(const Ray& ray) const {
    Vector3d dir = worldToLight_.apply(ray.dir()).normalized();
    Point2d st(vect::sphericalPhi(dir) * (0.5 * INV_PI), vect::sphericalTheta(dir) * INV_PI);
    return Spectrum(texmap_.lookup(st));
}

Spectrum Envmap::power() const {
    return PI * worldRadius_ * worldRadius_ * Spectrum(texmap_.lookup(Point2d(0.5, 0.5), 0.5));
}

Light* Envmap::clone() const {
    // TODO: doushiyoukana??
    return nullptr;
}

}  // namespace spica
