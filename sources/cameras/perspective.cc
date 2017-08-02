#define SPICA_API_EXPORT
#include "perspective.h"

#include <cmath>

#include "core/point2d.h"
#include "core/bounds2d.h"

#include "core/sampling.h"
#include "core/interaction.h"
#include "core/visibility_tester.h"
#include "core/film.h"

namespace spica {

PerspectiveCamera::PerspectiveCamera()
    : Camera{}
    , uCamera_{}
    , vCamera_{}
    , areaWorld_{0.0} {
}

PerspectiveCamera::PerspectiveCamera(RenderParams &params)
    : PerspectiveCamera{params.getTransform("toWorld", true),
                        Bounds2d(-1.0, -1.0, 1.0, 1.0),
                        params.getDouble("apertureRadius", true),
                        params.getDouble("focusDistance", true),
                        params.getDouble("fov", true) * PI / 180.0,
                        std::static_pointer_cast<Film>(params.getObject("film", true))} {
}

PerspectiveCamera::PerspectiveCamera(const Transform& cameraToWorld,
                                     const Bounds2d& screen, double lensRadius,
                                     double focalLength, double fov,
                                     std::shared_ptr<Film> film)
    : Camera{ cameraToWorld, 
              Transform::perspective(fov, film->aspect(), 1.0e-2, 1000.0),
              screen, lensRadius, focalLength, film }
    , uCamera_{}
    , vCamera_{}
    , areaWorld_{ 0.0 } {
    uCamera_ = rasterToCamera_.apply(Point3d(1.0, 0.0, 0.0)) - rasterToCamera_.apply(Point3d(0.0, 0.0, 0.0));
    vCamera_ = rasterToCamera_.apply(Point3d(0.0, 1.0, 0.0)) - rasterToCamera_.apply(Point3d(0.0, 0.0, 0.0));

    Point2i res = film->resolution();
    Point3d pMin = rasterToCamera_.apply(Point3d(0.0, 0.0, 0.0));
    Point3d pMax = rasterToCamera_.apply(Point3d(res.x(), res.y(), 0.0));
    pMin = { pMin.x() / pMin.z(), pMin.y() / pMin.z(), 1.0 };
    pMax = { pMax.x() / pMax.z(), pMax.y() / pMax.z(), 1.0 };
    areaWorld_ = std::abs((pMax.x() - pMin.x()) * (pMax.y() - pMin.y()));
}

Ray PerspectiveCamera::spawnRay(const Point2i& pixel, const Point2d& randFilm,
                                const Point2d& randLens, double* pdfPos,
                                double* pdfDir) const {
    Point3d pFilm = Point3d(pixel[0] + randFilm[0],
                            pixel[1] + randFilm[1], 0.0);
    Point3d pCamera = rasterToCamera_.apply(pFilm);
    
    Point3d  org = Point3d(0.0, 0.0, 0.0);
    Vector3d dir = vect::normalize(pCamera);
    if (lensRadius_ > 0.0) {
        Point2d pLens = lensRadius_ * sampleConcentricDisk(randLens);
        double ft = focalLength_ / dir.z();
        Point3d pFocus = org + dir * ft;

        org = Point3d(pLens.x(), pLens.y(), 0.0);
        dir = vect::normalize(pFocus - org);
    }

    Point3d  orgWorld = cameraToWorld_.apply(org);
    Vector3d dirWorld = cameraToWorld_.apply(dir);
    return Ray{ orgWorld, dirWorld };
}

Spectrum PerspectiveCamera::We(const Ray& ray, Point2d* pRaster) const {
    const Transform& c2w = cameraToWorld_;
    double cosTheta = vect::dot(ray.dir(), c2w.apply(Vector3d(0.0, 0.0, 1.0)));
    if (cosTheta <= 0.0) return Spectrum(0.0);

    Point3d pFocus   = ray.proceeded((lensRadius_ > 0.0 ? focalLength_ : 1.0) / cosTheta);
    Point3d pRaster3 = rasterToCamera_.inverted().apply(c2w.inverted().apply(pFocus));

    if (pRaster) *pRaster = Point2d(pRaster3.x(), pRaster3.y());

    Bounds2i sampleBounds(0, 0, film_->resolution().x(), film_->resolution().y());
    if (pRaster->x() < sampleBounds.posMin().x() || pRaster->x() >= sampleBounds.posMax().x() ||
        pRaster->y() < sampleBounds.posMin().y() || pRaster->y() >= sampleBounds.posMax().y()) {
        return Spectrum(0.0);
    }

    double lensArea = lensRadius_ != 0.0 ? (PI * lensRadius_ * lensRadius_) : 1.0;

    double cos2Theta = cosTheta * cosTheta;
    return Spectrum(1.0 / (areaWorld_ * lensArea * cos2Theta * cos2Theta));
}

void PerspectiveCamera::pdfWe(const Ray& ray, double* pdfPos, double* pdfDir) const {
    const Transform& c2w = cameraToWorld_;
    double cosTheta = vect::dot(ray.dir(), c2w.apply(Vector3d(0.0, 0.0, 1.0)));
    if (cosTheta <= 0.0) {
        *pdfPos = *pdfDir = 0.0;
        return;
    }

    Point3d pFocus   = ray.proceeded((lensRadius_ > 0.0 ? focalLength_ : 1.0) / cosTheta);
    Point3d pRaster = rasterToCamera_.inverted().apply(c2w.inverted().apply(pFocus));

    Bounds2i sampleBounds(0, 0, film_->resolution().x(), film_->resolution().y());
    if (pRaster.x() < sampleBounds.posMin().x() || pRaster.x() >= sampleBounds.posMax().x() ||
        pRaster.y() < sampleBounds.posMin().y() || pRaster.y() >= sampleBounds.posMax().y()) {
        *pdfPos = *pdfDir = 0.0;
        return;
    }

    double lensArea = lensRadius_ != 0.0 ? (PI * lensRadius_ * lensRadius_) : 1.0;
    *pdfPos = 1.0 / lensArea;
    *pdfDir = 1.0 / (areaWorld_ * cosTheta * cosTheta * cosTheta);
}

Spectrum PerspectiveCamera::sampleWi(const Interaction& ref, const Point2d& rand,
                                     Vector3d* wi, double* pdf, Point2d* pRaster,
                                     VisibilityTester* vis) const {
    Point2d pLens = lensRadius_ * sampleConcentricDisk(rand);
    Point3d pLensWorld = cameraToWorld_.apply(Point3d(pLens.x(), pLens.y(), 0.0));
    Normal3d nLensWorld = Normal3d(cameraToWorld_.apply(Vector3d(0.0, 0.0, 1.0)));
    Interaction lensIntr(pLensWorld, nLensWorld);

    *vis = VisibilityTester(ref, lensIntr);
    *wi  = lensIntr.pos() - ref.pos();
    double dist = wi->norm();
    *wi /= dist;

    double lensArea = lensRadius_ != 0.0 ? (PI * lensRadius_ * lensRadius_) : 1.0;
    *pdf = (dist * dist) / (vect::absDot(lensIntr.normal(), *wi) * lensArea);
    return We(lensIntr.spawnRay(-*wi), pRaster);
}

}  // namespace spica
