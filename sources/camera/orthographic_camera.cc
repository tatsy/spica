#define SPICA_API_EXPORT
#include "orthographic_camera.h"

#include "../core/point2d.h"

namespace spica {

    OrthographicCamera::OrthographicCamera()
        : ICamera() {
    }

    OrthographicCamera::OrthographicCamera(const Point& center,
                                       const Vector3D& direction,
                                       const Vector3D& up,
                                       const Rect& rect,
                                       int imageW, int imageH, double sensitivity)
        : ICamera{center, direction, up, imageW, imageH, sensitivity}
        , _rect{rect} {
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

    CameraSample OrthographicCamera::sample(double px, double py, const Point2D& rands) const {
        const Vector3D vecU = _rect.width()  * (((_imageW - px - 1) + rands[0]) / _imageW - 0.5) * _unitU;
        const Vector3D vecV = _rect.height() * ((py + rands[1]) / _imageH - 0.5) * _unitV;
        const Point    origin = _center + vecU + vecV;
        return CameraSample(Ray(origin, _direction), 1.0);
    }

    ICamera* OrthographicCamera::clone() const {
        return new OrthographicCamera(*this);
    }

}  // namespace spica
