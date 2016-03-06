#define SPICA_API_EXPORT
#include "perspective.h"

#include <cmath>

#include "../core/point2d.h"
#include "../core/sampling.h"
#include "../image/film.h"

namespace spica {

PerspectiveCamera::PerspectiveCamera()
    : Camera{}
    , uCamera_{}
    , vCamera_{}
    , areaWorld_{0.0} {
}

PerspectiveCamera::PerspectiveCamera(const Transform& cameraToWorld,
                                     const RectF& screen, double lensRadius,
                                     double focalLength, double fov,
                                     Film* film)
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

}  // namespace spica
