#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_CAMERA_INTERFACE_H_
#define _SPICA_CAMERA_INTERFACE_H_

#include "../core/common.h"
#include "../math/transform.h"

namespace spica {

class SPICA_EXPORTS Camera {
public:
    // Public methods
    Camera();
    Camera(const Transform& cameraToWorld,
           const Transform& cameraToScreen,
           const RectF& screen,
           double lensRadius, double focalLength,
           Film* film);
    Camera(const Camera&) = default;

    virtual ~Camera();

    Camera& operator=(const Camera&) = default;

    virtual Ray spawnRay(const Point2i& pixel, const Point2D& randFilm,
                         const Point2D& randLens, double* pdfPos = nullptr,
                         double* pdfDir = nullptr) const = 0;

    inline Film* film() const { return film_; }

protected:
    // Private fields
    Transform cameraToWorld_;
    Transform cameraToScreen_, rasterToCamera_;
    Transform screenToRaster_, rasterToScreen_;
    double lensRadius_, focalLength_;
    Film* film_;
};

}  // namespace spica

#endif  // _SPICA_CAMERA_INTERFACE_H_
