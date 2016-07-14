#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ORTHOGRAPHIC_CAMERA_H_
#define _SPICA_ORTHOGRAPHIC_CAMERA_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "camera.h"

namespace spica {

/** Orthographic camera.
 *  @ingroup camera_module
 */
class SPICA_EXPORTS OrthographicCamera : public Camera {
public:
    // Public methods
    OrthographicCamera();
    OrthographicCamera(const Transform& cameraToWorld,
                        const Bounds2d& screen, double lensRadius,
                        double focalLength, Film* film);
    OrthographicCamera(const OrthographicCamera&) = default;
    ~OrthographicCamera() = default;

    OrthographicCamera& operator=(const OrthographicCamera&) = default;

    Ray spawnRay(const Point2i& pixel, const Point2d& randFilm,
                 const Point2d& randLens, double* pdfPos = nullptr,
                 double* pdfDir = nullptr) const override;

    Spectrum We(const Ray& ray, Point2d* pRaster = nullptr) const override;
    void pdfWe(const Ray& ray, double* pdfPos, double* pdfDir) const override;
    Spectrum sampleWi(const Interaction& ref, const Point2d& rand,
                      Vector3d* wi, double* pdf, Point2d* pRaster,
                      VisibilityTester* vis) const override;

private:
    // Private fields
    Vector3d uCamera_, vCamera_;

};  // class OrthographicCamera

}  // namespace spica

#endif  // _SPICA_ORTHOGRAPHIC_CAMERA_H_
