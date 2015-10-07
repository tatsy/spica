#define SPICA_PERSPECTIVE_CAMERA_EXPORT
#include "perspective_camera.h"

#include <cmath>

namespace spica {

    PerspectiveCamera::PerspectiveCamera()
        : ICamera()
        , _fov(0.0)
        , _aspect(0.0)
        , _halfTangent(0.0) {
    }

    PerspectiveCamera::PerspectiveCamera(const Vector3D& center,
                                         const Vector3D& direction,
                                         const Vector3D& up,
                                         double fov, int imageW, int imageH,
                                         double sensitivity)
        : ICamera(center, direction, up, imageW, imageH, sensitivity)
        , _fov(fov)
        , _aspect((double)imageW / (double)imageH)
        , _halfTangent(tan(fov * 0.5)) {
    }

    PerspectiveCamera::~PerspectiveCamera() {
    }

    PerspectiveCamera::PerspectiveCamera(const PerspectiveCamera& camera)
        : ICamera()
        , _fov(0.0)
        , _aspect(0.0)
        , _halfTangent(0.0) {
        this->operator=(camera);    
    }

    PerspectiveCamera& PerspectiveCamera::operator=(const PerspectiveCamera& camera) {
        ICamera::operator=(camera);
        this->_fov = camera._fov;
        this->_aspect = camera._aspect;
        this->_halfTangent = camera._halfTangent;
        return *this;
    }

    CameraSample PerspectiveCamera::sample(double px, double py, Stack<double>& rstk) const {
        const Vector3D vecU = _halfTangent * _aspect * (2.0 * px / _imageW - 1.0) * _unitU;
        const Vector3D vecV = _halfTangent * (2.0 * py / _imageH - 1.0) * _unitV;
        Vector3D dir = _direction + vecU + vecV;
        return CameraSample(Ray(_center, dir), 1.0);
    }

    ICamera* PerspectiveCamera::clone() const {
        return new PerspectiveCamera(*this);
    }

}  // namespace spica
