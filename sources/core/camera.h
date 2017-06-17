#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_CAMERA_INTERFACE_H_
#define _SPICA_CAMERA_INTERFACE_H_

#include "core/common.h"
#include "core/transform.h"
#include "core/cobject.h"

#include "core/render.hpp"

namespace spica {

class SPICA_EXPORTS Camera : public CObject {
public:
    // Public methods
    Camera();
    Camera(const Transform& cameraToWorld,
           const Transform& cameraToScreen,
           const Bounds2d& screen,
           double lensRadius, double focalLength,
           std::shared_ptr<Film> film);
    Camera(const Camera&) = default;

    virtual ~Camera();

    Camera& operator=(const Camera&) = default;

    virtual Ray spawnRay(const Point2i& pixel, const Point2d& randFilm,
                         const Point2d& randLens, double* pdfPos = nullptr,
                         double* pdfDir = nullptr) const = 0;

    virtual Spectrum We(const Ray& ray, Point2d* pRaster = nullptr) const = 0;
    virtual void pdfWe(const Ray& ray, double* pdfPos, double* pdfDir) const = 0;
    virtual Spectrum sampleWi(const Interaction& ref, const Point2d& rand,
                              Vector3d* wi, double* pdf, Point2d* pRaster,
                              VisibilityTester* vis) const = 0;

    inline std::shared_ptr<Film> film() const { return film_; }
    inline const Transform& cameraToWorld() const { return cameraToWorld_; }
    inline const Transform& cameraToScreen() const { return cameraToScreen_; }

protected:
    // Private fields
    Transform cameraToWorld_;
    Transform cameraToScreen_, rasterToCamera_;
    Transform screenToRaster_, rasterToScreen_;
    double lensRadius_, focalLength_;
    std::shared_ptr<Film> film_;
};

}  // namespace spica

#endif  // _SPICA_CAMERA_INTERFACE_H_
