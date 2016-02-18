#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PERSPECTIVE_CAMERA_H_
#define _SPICA_PERSPECTIVE_CAMERA_H_

#include "../math/vector3d.h"

#include "camera.h"

namespace spica {

    /** Perspective camera
     * @ingroup camera_module
     */
    class SPICA_EXPORTS PerspectiveCamera : public Camera {
    public:
        // Public methods
        PerspectiveCamera();
        PerspectiveCamera(const Transform& cameraToWorld,
                          const RectF& screen, double lensRadius,
                          double focalLength, double fov, Film* film);

        PerspectiveCamera(const PerspectiveCamera&) = default;
        ~PerspectiveCamera();

        PerspectiveCamera& operator=(const PerspectiveCamera&) = default;

        Ray spawnRay(const Point2D& randFilm,
                     const Point2D& randLens) const override;

    private:
        // Private fields
        Vector3D uCamera_, vCamera_;
        double areaWorld_;
    };

}  // namespace spica

#endif  // _SPICA_PERSPECTIVE_CAMERA_H_
