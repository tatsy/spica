#define SPICA_API_EXPORT
#include "envmap.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "core/common.h"
#include "core/point2d.h"
#include "core/normal3d.h"
#include "core/sampling.h"
#include "core/scene.h"
#include "core/visibility_tester.h"

namespace spica {

Envmap::Envmap(const BSphere& worldSphere, const Image& texmap, const Transform& lightToWorld,
               double scale, int numSamples)
    : Light{ LightType::Envmap, Transform{lightToWorld.getMat().transposed()}, numSamples }
    , mipmap_{ nullptr }
    , worldCenter_{ worldSphere.center() }
    , worldRadius_{ worldSphere.radius() }
    , distrib_{} {
    const int width = texmap.width();
    const int height = texmap.height();

    Image tmap(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tmap.pixel(x, y) = texmap(x, y) * scale;
        }
    }
    mipmap_ = std::make_unique<MipMap>(tmap, ImageWrap::Repeat);

    const double filter = 1.0 / std::max(width, height);
    std::vector<double> gray(width * height);
    for (int v = 0; v < height; v++) {
        double vp = static_cast<double>(v + 0.5) / height;
        double sinTheta = std::sin(PI * (v + 0.5) / height);
        for (int u = 0; u < width; u++) {
            double up = static_cast<double>(u + 0.5) / width;
            gray[v * width + u] = mipmap_->lookup(Point2d(up, vp), filter).gray();
            gray[v * width + u] *= sinTheta;
        }
    }
    distrib_ = Distribution2D(gray, width, height);
}

Envmap::Envmap(RenderParams &params)
    : Envmap{BSphere(params.getPoint3d("worldCenter", Point3d(0.0, 0.0, 0.0), true),
                     params.getDouble("worldRadius", 2.0, true)),
             Image::fromFile(params.getString("filename", true)),
             params.getTransform("toWorld", Transform{}, true),
             params.getDouble("scale", 1.0, true)} {
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
    const double cosTheta = std::cos(theta);
    const double sinTheta = std::sin(theta);
    const double cosPhi   = std::cos(phi);
    const double sinPhi   = std::sin(phi);
    *dir = lightToWorld_.apply(Vector3d(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta));
    *pdf = sinTheta == 0.0 ? 0.0 : mapPdf / (2.0 * PI * PI * sinTheta);

    Interaction pLight(pObj.pos() + (*dir) * (2.0 * worldRadius_));
    *vis = VisibilityTester(pObj, pLight);
        
    return Spectrum(mipmap_->lookup(uv));
}

double Envmap::pdfLi(const Interaction&, const Vector3d& d) const {
    Vector3d dir = worldToLight_.apply(d);
    double theta = vect::sphericalTheta(dir);
    double phi   = vect::sphericalPhi(dir);
    double sinTheta = sin(theta);
    if (sinTheta == 0.0) return 0.0;
    return distrib_.pdf(Point2d(phi * (0.5 * INV_PI), theta * INV_PI)) / (2.0 * PI * PI * sinTheta);
}

Spectrum Envmap::sampleLe(const Point2d& rand1, const Point2d& rand2,
                          Ray* ray, Normal3d* nLight, double* pdfPos,
                          double* pdfDir) const {
    double mapPdf;
    Point2d uv = distrib_.sample(rand1, &mapPdf);
    if (mapPdf == 0.0) return Spectrum(0.0);

    double theta = uv[1] * PI;
    double phi   = uv[0] * 2.0 * PI;
    double cosTheta = std::cos(theta);
    double sinTheta = std::sin(theta);
    double sinPhi   = std::sin(phi);
    double cosPhi   = std::cos(phi);
    Vector3d d =
        -lightToWorld_.apply(Vector3d(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta));
    *nLight = Normal3d(d);

    Vector3d v1, v2;
    vect::coordinateSystem(-d, &v1, &v2);
    Point2d cd = sampleConcentricDisk(rand2);
    Point3d pDisk = worldCenter_ + worldRadius_ * (cd.x() * v1 + cd.y() * v2);
    *ray = Ray(pDisk - worldRadius_ * d, d, INFTY);

    *pdfDir = sinTheta == 0.0 ? 0.0 : mapPdf / (2.0 * PI * PI * sinTheta);
    *pdfPos = 1.0 / (PI * worldRadius_ * worldRadius_);
    return Spectrum(mipmap_->lookup(uv));
}


void Envmap::pdfLe(const Ray& ray, const Normal3d& nLight, double* pdfPos,
                   double* pdfDir) const {
    Vector3d d = -worldToLight_.apply(ray.dir());
    double theta = std::acos(clamp(d.z(), -1.0, 1.0));
    double phi   = std::atan2(d.y(), d.x());
    if (phi < 0.0) phi += 2.0 * PI;
    Point2d uv(phi * (0.5 * INV_PI), theta * INV_PI);
    double mapPdf = distrib_.pdf(uv);
    *pdfDir = mapPdf / (2.0 * PI * PI * std::sin(theta));
    *pdfPos = 1.0 / (PI * worldRadius_ * worldRadius_);
}

Spectrum Envmap::Le(const Ray& ray) const {
    Vector3d dir = worldToLight_.apply(ray.dir()).normalized();
    Point2d st(vect::sphericalPhi(dir) * (0.5 * INV_PI), vect::sphericalTheta(dir) * INV_PI);
    return Spectrum(mipmap_->lookup(st));
}

Spectrum Envmap::power() const {
    return PI * worldRadius_ * worldRadius_ * Spectrum(mipmap_->lookup(Point2d(0.5, 0.5), 0.5));
}

Light* Envmap::clone() const {
    FatalError("Envmap::clone() is not implemented!");
    return nullptr;
}

}  // namespace spica
