#define SPICA_CAMERA_EXPORT
#include "Camera.h"

namespace spica {

    // ------------------------------------------------------------
    // Image sensor
    // ------------------------------------------------------------

    ImageSensor::ImageSensor()
        : _width(0.0)
        , _height(0.0)
        , _center()
        , _direction()
        , _up()
        , _u()
        , _v()
        , _sensitivity(0.0)
    {
    }

    ImageSensor::ImageSensor(double width, double height, Vector3 center, Vector3 direction, Vector3 up, Vector3 u, Vector3 v, double sensitivity)
        : _width(width)
        , _height(height)
        , _center(center)
        , _direction(direction)
        , _up(up)
        , _u(u)
        , _v(v)
        , _sensitivity(sensitivity)
    {
    }

    ImageSensor::ImageSensor(const ImageSensor& sensor)
        : _width(sensor._width)
        , _height(sensor._height)
        , _center(sensor._center)
        , _direction(sensor._direction)
        , _up(sensor._up)
        , _u(sensor._u)
        , _v(sensor._v)
        , _sensitivity(sensor._sensitivity)
    {
    }

    ImageSensor::~ImageSensor()
    {
    }

    ImageSensor& ImageSensor::operator=(const ImageSensor& sensor) {
        this->_width       = sensor._width;
        this->_height      = sensor._height;
        this->_center      = sensor._center;
        this->_direction   = sensor._direction;
        this->_up          = sensor._up;
        this->_u           = sensor._u;
        this->_v           = sensor._v;
        this->_sensitivity = sensor._sensitivity;
        return *this;
    }


    // ------------------------------------------------------------
    // Lens
    // ------------------------------------------------------------

    Lens::Lens()
        : _focalLength(0.0)
        , _radius(0.0)
        , _center()
        , _u()
        , _v()
        , _normal()
    {
    }

    Lens::Lens(double focalLength, double radius, Vector3 center, Vector3 u, Vector3 v, Vector3 normal)
        : _focalLength(focalLength)
        , _radius(radius)
        , _center(center)
        , _u(u)
        , _v(v)
        , _normal(normal)
    {
    }

    Lens::Lens(const Lens& lens)
        : _focalLength(lens._focalLength)
        , _radius(lens._radius)
        , _center(lens._center)
        , _u(lens._u)
        , _v(lens._v)
        , _normal(lens._normal)
    {
    }

    Lens::~Lens()
    {
    }

    Lens& Lens::operator=(const Lens& lens) {
        this->_focalLength = lens._focalLength;
        this->_radius = lens._radius;
        this->_center = lens._center;
        this->_u = lens._u;
        this->_v = lens._v;
        this->_normal = lens._normal;
        return *this;
    }
}

