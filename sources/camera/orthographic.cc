#define SPICA_API_EXPORT
#include "orthographic.h"

#include "../core/point2d.h"
#include "../core/sampling.h"

namespace spica {

OrthographicCamera::OrthographicCamera()
    : uCamera_{}
    , vCamera_{} {
}

OrthographicCamera::OrthographicCamera(const Transform& cameraToWorld,
                                       const Bounds2d& screen, double lensRadius,
                                       double focalLength, Film* film)
    : Camera{ cameraToWorld, Transform::orthographic(0.0, 1.0), screen,
              lensRadius, focalLength, film }
    , uCamera_{}
    , vCamera_{} {
    uCamera_ = rasterToCamera_.apply(Vector3d(1.0, 0.0, 0.0));
    vCamera_ = rasterToCamera_.apply(Vector3d(0.0, 1.0, 0.0));
}

Ray OrthographicCamera::spawnRay(const Point2i& pixel, const Point2d& randFilm,
                                 const Point2d& randLens, double* pdfPos,
                                 double* pdfDir) const {
    Point3d pFilm(pixel[0] + randFilm[0], pixel[1] + randFilm[1], 0.0);
    Point3d pCamera = rasterToCamera_.apply(pFilm);
    
    Point3d  org = pCamera;
    Vector3d dir = Vector3d(0.0, 0.0, 1.0);
    if (lensRadius_ > 0.0) {
        Point2d pLens = lensRadius_ * sampleConcentricDisk(randLens);

        double ft = focalLength_ / dir.z();
        Point3d pFocus = org + ft * dir;

        org = Point3d(pLens.x(), pLens.y(), 0.0);
        dir = (pFocus - org).normalized();
    }

    Point3d orgWorld  = cameraToWorld_.apply(org);
    Vector3d dirWorld = cameraToWorld_.apply(dir);
    return Ray{ orgWorld, dirWorld };
}

}  // namespace spica
