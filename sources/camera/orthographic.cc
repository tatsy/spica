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
                                       const RectF& screen, double lensRadius,
                                       double focalLength, Film* film)
    : Camera{ cameraToWorld, Transform::orthographic(0.0, 1.0), screen,
              lensRadius, focalLength, film }
    , uCamera_{}
    , vCamera_{} {
    uCamera_ = rasterToCamera_.apply(Vector3D(1.0, 0.0, 0.0));
    vCamera_ = rasterToCamera_.apply(Vector3D(0.0, 1.0, 0.0));
}

Ray OrthographicCamera::spawnRay(const Point2i& pixel, const Point2D& randFilm,
                                 const Point2D& randLens, double* pdfPos,
                                 double* pdfDir) const {
    Point3D pFilm(pixel[0] + randFilm[0], pixel[1] + randFilm[1], 0.0);
    Point3D pCamera = rasterToCamera_.apply(pFilm);
    
    Point3D  org = pCamera;
    Vector3D dir = Vector3D(0.0, 0.0, 1.0);
    if (lensRadius_ > 0.0) {
        Point2D pLens = lensRadius_ * sampleConcentricDisk(randLens);

        double ft = focalLength_ / dir.z();
        Point3D pFocus = org + ft * dir;

        org = Point3D(pLens.x(), pLens.y(), 0.0);
        dir = (pFocus - org).normalized();
    }

    Point3D orgWorld  = cameraToWorld_.apply(org);
    Vector3D dirWorld = cameraToWorld_.apply(dir);
    return Ray{ orgWorld, dirWorld };
}

}  // namespace spica
