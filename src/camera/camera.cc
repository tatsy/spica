#define SPICA_CAMERA_EXPORT
#include "camera.h"

#include <memory>

#include "dof_camera.h"
#include "perspective_camera.h"
#include "orthographic_camera.h"

namespace spica {

    Camera::Camera()
        : _ptr(nullptr) {
    }

    Camera::~Camera() {
        delete _ptr;
    }

    Camera::Camera(const Camera& camera)
        : _ptr(nullptr) {
        this->operator=(camera);
    }

    Camera::Camera(Camera&& camera)
        : _ptr(nullptr) {
        this->operator=(std::move(camera));
    }

    Camera& Camera::operator=(const Camera& camera) {
        this->_ptr = camera._ptr->clone();
        this->_type = camera._type;
        return *this;
    }

    Camera& Camera::operator=(Camera&& camera) {
        this->_ptr  = camera._ptr;
        this->_type = camera._type;

        camera._ptr = nullptr;
        return *this;
    }

    CameraSample Camera::sample(double px, double py, Stack<double>& rstack) const {
        return _ptr->sample(px, py, rstack);
    }

    Camera Camera::asDoF(int imageWidth,
                         int imageHeight,
                         const  Vector3D& sensorCenter,
                         const  Vector3D& sensorDir,
                         const  Vector3D& sensorUp,
                         double sensorSize,
                         double distSensorToLens,
                         double focalLength,
                         double lensRadius,
                         double sensorSensitivity) {

        Camera camera;
        camera._type = CameraType::DepthOfField;
        camera._ptr = new DoFCamera(imageWidth,
                                    imageHeight,
                                    sensorCenter,
                                    sensorDir,
                                    sensorUp,
                                    sensorSize,
                                    distSensorToLens,
                                    focalLength,
                                    lensRadius,
                                    sensorSensitivity);
        return std::move(camera);
    }

    Camera Camera::perspective(const Vector3D& center,
                               const Vector3D& direction,
                               const Vector3D& up,
                               double fov, int imageW, int imageH,
                               double sensitivity) {
        Camera camera;
        camera._type = CameraType::Perspective;
        camera._ptr = new PerspectiveCamera(center,
                                            direction,
                                            up,
                                            fov, imageW, imageH,
                                            sensitivity);
        return std::move(camera);
    }

    Camera Camera::orthogonal(const Vector3D& center,
                              const Vector3D& direction,
                              const Vector3D& up,
                              int imageW, int imageH, double sensitivity) {
        Camera camera;
        camera._type = CameraType::Orthogonal;
        camera._ptr = new OrthographicCamera(center, direction, up,
                                           imageW, imageH, sensitivity);
        return std::move(camera);
    }

    std::unique_ptr<ICamera> Camera::ptr() const {
        return std::move(std::unique_ptr<ICamera>(_ptr->clone()));
    }

}  // namespace spica
