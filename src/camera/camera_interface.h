#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_CAMERA_INTERFACE_H_
#define _SPICA_CAMERA_INTERFACE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #define SPICA_CAMERA_INTERFACE_DLL __declspec(dllexport)
#else
    #define SPICA_CAMERA_INTERFACE_DLL
#endif

#include <cstdio>

#include "../utils/vector3d.h"
#include "../utils/stack.h"
#include "../renderer/ray.h"

namespace spica {

    class SPICA_CAMERA_INTERFACE_DLL CameraSample {
    private:
        Ray    _ray;
        double _pdf;

    public:
        CameraSample()
            : _ray()
            , _pdf(0.0) {
        }

        CameraSample(const Ray& ray, double pdf)
            : _ray(ray)
            , _pdf(pdf) {
        }

        CameraSample(const CameraSample& cs)
            : _ray(cs._ray)
            , _pdf(cs._pdf) {
        }

        ~CameraSample() {
        }

        CameraSample& operator=(const CameraSample& cs) {
            this->_ray = cs._ray;
            this->_pdf = cs._pdf;
            return *this;
        }
        
        inline const Ray& ray() const { return _ray; }
        inline double     pdf() const { return _pdf; }
    };

    //! Interface class for cameras
    class SPICA_CAMERA_INTERFACE_DLL ICamera {
    protected:
        Vector3D _center;
        Vector3D _direction;
        Vector3D _up;
        Vector3D _unitU;
        Vector3D _unitV;
        int _imageW;
        int _imageH;
        double _sensitivity;

    public:
        ICamera()
            : _center()
            , _direction()
            , _up()
            , _unitU()
            , _unitV()
            , _imageW(0)
            , _imageH(0)
            , _sensitivity(0.0) {
        }

        ICamera(const Vector3D& center,
                const Vector3D& direction,
                const Vector3D& up,
                int imageW, int imageH,
                double sensitivity)
            : _center(center)
            , _direction(direction)
            , _up(up)
            , _unitU()
            , _unitV()
            , _imageW(imageW)
            , _imageH(imageH)
            , _sensitivity(sensitivity) {
            // Compute unit basis vectors
            _unitU = direction.cross(up).normalized();
            _unitV = _unitU.cross(direction).normalized();
        }

        virtual ~ICamera() {}

        ICamera& operator=(const ICamera& camera) {
            this->_center    = camera._center;
            this->_direction = camera._direction;
            this->_up        = camera._up;
            this->_imageW    = camera._imageW;
            this->_imageH    = camera._imageH;
            return *this;
        }

        virtual ICamera* clone() const = 0;
        virtual CameraSample sample(double px, double py, Stack<double>& rstack) const = 0;

        inline virtual Vector3D center()    const { return _center; }
        inline virtual Vector3D direction() const { return _direction; }
        inline virtual Vector3D up() const { return _up; }
        inline int imageW() const { return _imageW; }
        inline int imageH() const { return _imageH; }
        inline virtual double sensitivity() const { return _sensitivity; }
    };

}  // namespace spica

#endif  // _SPICA_CAMERA_INTERFACE_H_
