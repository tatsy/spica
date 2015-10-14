#define SPICA_API_EXPORT
#include "orthographic_camera.h"

namespace spica {

    OrthographicCamera::OrthographicCamera()
        : ICamera() {
    }

    OrthographicCamera::OrthographicCamera(const Vector3D& center,
                                       const Vector3D& direction,
                                       const Vector3D& up,
                                       int imageW, int imageH, double sensitivity)
        : ICamera(center, direction, up, imageW, imageH, sensitivity) {
    }

    OrthographicCamera::~OrthographicCamera() {
    }

    OrthographicCamera::OrthographicCamera(const OrthographicCamera& camera)
        : ICamera() {
        this->operator=(camera);
    }

    OrthographicCamera& OrthographicCamera::operator=(const OrthographicCamera& camera) {
        ICamera::operator=(camera);
        return *this;
    }

    CameraSample OrthographicCamera::sample(double px, double py, Stack<double>& rstk) const {
        const double invS = 1.0 / std::max(_imageW, _imageH);
        const Vector3D vecU = _rect.width() * (2.0 * px * invS - 1.0) * _unitU;
        const Vector3D vecV = _rect.height() * (2.0 * py * invS - 1.0) * _unitV;
        const Vector3D origin = _center + vecU + vecV;
        return CameraSample(Ray(origin, _direction), 1.0);
    }

    ICamera* OrthographicCamera::clone() const {
        return new OrthographicCamera(*this);
    }

}  // namespace spica
