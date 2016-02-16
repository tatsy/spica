#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_CAMERA_INTERFACE_H_
#define _SPICA_CAMERA_INTERFACE_H_

#include <cstdio>

#include "../math/vector3d.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"
#include "../core/stack.h"
#include "../core/ray.h"

namespace spica {

    class SPICA_EXPORTS CameraSample {
    public:
        CameraSample()
            : ray_{}
            , pdf_{ 0.0 }
            , posSensor_{}
            , posObjplene_{} 
            , posLens_{} {
        }

        CameraSample(const Ray& ray, double pdf,
                     const Point& posSensor   = Point(0.0, 0.0, 0.0),
                     const Point& posObjplane = Point(0.0, 0.0, 0.0),
                     const Point& posLens     = Point(0.0, 0.0, 0.0))
            : ray_{ ray }
            , pdf_{ pdf }
            , posSensor_{ posSensor }
            , posObjplene_{ posObjplane }
            , posLens_{ posLens } {
        }

        CameraSample(const CameraSample& cs)
            : ray_{ cs.ray_ }
            , pdf_{ cs.pdf_ }
            , posSensor_{ cs.posSensor_ }
            , posObjplene_{ cs.posObjplene_ }
            , posLens_{ cs.posLens_ } {
        }

        ~CameraSample() {
        }

        CameraSample& operator=(const CameraSample& cs) {
            this->ray_ = cs.ray_;
            this->pdf_ = cs.pdf_;
            this->posSensor_ = cs.posSensor_;
            this->posObjplene_ = cs.posObjplene_;
            this->posLens_ = cs.posLens_;
            return *this;
        }
        
        inline const Ray& ray() const { return ray_; }
        inline double     pdf() const { return pdf_; }
        inline const Point& posSensor() const { return posSensor_; }
        inline const Point& posObjplane() const { return posObjplene_; }
        inline const Point& posLens() const { return posLens_; }

    private:
        Ray    ray_;
        double pdf_;
        Point  posSensor_;
        Point  posObjplene_;
        Point  posLens_;
    };

    //! Interface class for cameras
    class SPICA_EXPORTS ICamera {
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

        ICamera(const Point& center,
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
            this->_center      = camera._center;
            this->_direction   = camera._direction;
            this->_up          = camera._up;
            this->_imageW      = camera._imageW;
            this->_imageH      = camera._imageH;
            this->_sensitivity = camera._sensitivity;
            return *this;
        }

        virtual ICamera* clone() const = 0;
        virtual CameraSample sample(double px, double py, const Point2D& rands) const = 0;

        inline virtual Point    center()    const { return _center; }
        inline virtual Vector3D direction() const { return _direction; }
        inline virtual Vector3D up() const { return _up; }
        inline int imageW() const { return _imageW; }
        inline int imageH() const { return _imageH; }
        inline virtual double sensitivity() const { return _sensitivity; }

    protected:
        Point _center;
        Vector3D _direction;
        Vector3D _up;
        Vector3D _unitU;
        Vector3D _unitV;
        int _imageW;
        int _imageH;
        double _sensitivity;

    };

}  // namespace spica

#endif  // _SPICA_CAMERA_INTERFACE_H_
