#define SPICA_ORTHOGONAL_CAMERA_EXPORT
#include "orthogonal_camera.h"

namespace spica {

    OrthogonalCamera::OrthogonalCamera()
        : ICamera() {
    }

    OrthogonalCamera::OrthogonalCamera(const Vector3D& center,
                                       const Vector3D& direction,
                                       const Vector3D& up,
                                       int imageW, int imageH, double sensitivity)
        : ICamera(center, direction, up, imageW, imageH, sensitivity) {
    }

    OrthogonalCamera::~OrthogonalCamera() {
    }

    OrthogonalCamera::OrthogonalCamera(const OrthogonalCamera& camera)
        : ICamera() {
        this->operator=(camera);
    }

    OrthogonalCamera& OrthogonalCamera::operator=(const OrthogonalCamera& camera) {
        ICamera::operator=(camera);
        return *this;
    }

    CameraSample OrthogonalCamera::sample(double px, double py, Stack<double>& rstk) const {
        const double invS = 1.0 / std::max(_imageW, _imageH);
        const Vector3D vecU = _rect.width() * (2.0 * px * invS - 1.0) * _unitU;
        const Vector3D vecV = _rect.height() * (2.0 * py * invS - 1.0) * _unitV;
        const Vector3D origin = _center + vecU + vecV;
        return CameraSample(Ray(origin, _direction), 1.0);
    }

    ICamera* OrthogonalCamera::clone() const {
        return new OrthogonalCamera(*this);
    }

}  // namespace spica
