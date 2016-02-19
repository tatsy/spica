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
                        const RectF& screen, double lensRadius,
                        double focalLength, Film* film);
    OrthographicCamera(const OrthographicCamera&) = default;
    ~OrthographicCamera() = default;

    OrthographicCamera& operator=(const OrthographicCamera&) = default;

    Ray spawnRay(const Point2i& pixel, const Point2D& randFilm,
                 const Point2D& randLens, double* pdfPos = nullptr,
                 double* pdfDir = nullptr) const override;
private:
    // Private fields
    Vector3D uCamera_, vCamera_;

};  // class OrthographicCamera

}  // namespace spica

#endif  // _SPICA_ORTHOGRAPHIC_CAMERA_H_
