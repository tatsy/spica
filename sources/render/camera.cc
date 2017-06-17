#define SPICA_API_EXPORT
#include "camera.h"

#include "core/bounds2d.h"
#include "render/film.h"

namespace spica {

Camera::Camera()
    : cameraToWorld_{}
    , cameraToScreen_{}
    , rasterToCamera_{}
    , screenToRaster_{}
    , rasterToScreen_{}
    , lensRadius_{ 0.0 }
    , focalLength_{ 0.0 }
    , film_{ nullptr } {
}

Camera::Camera(const Transform& cameraToWorld, const Transform& cameraToScreen,
               const Bounds2d& screen, double lensRadius, double focalLength,
               Film* film)
    : cameraToWorld_{ cameraToWorld }
    , cameraToScreen_{ cameraToScreen }
    , rasterToCamera_{}
    , screenToRaster_{}
    , rasterToScreen_{}
    , lensRadius_{ lensRadius }
    , focalLength_{ focalLength }
    , film_{ film } {
    Point2i res = film_->resolution();
    screenToRaster_ = Transform::scale(res.x(), res.y(), 1.0) *
                      Transform::scale( 1.0 / screen.width(),
                                       -1.0 / screen.height(), 1.0) *
                      Transform::translate(Vector3d(-screen.posMin().x(), -screen.posMax().y(), 0.0));
    rasterToScreen_ = screenToRaster_.inverted();
    rasterToCamera_ = cameraToScreen_.inverted() * rasterToScreen_;
}

Camera::~Camera() {
}

}  // namespace spica