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


    // ------------------------------------------------------------
    // Object plane
    // ------------------------------------------------------------

    ObjectPlane::ObjectPlane()
        : Plane()
        , _center()
        , _u()
        , _v()
    {
    }

    ObjectPlane::ObjectPlane(Vector3 center, Vector3 u, Vector3 v)
        : Plane()
        , _center(center)
        , _u(u)
        , _v(v)
    {

        this->_normal = v.cross(u).normalize();
        this->_distance = - _normal.dot(center);
    }

    ObjectPlane::ObjectPlane(const ObjectPlane& objplane)
        : Plane(objplane)
        , _center(objplane._center)
        , _u(objplane._u)
        , _v(objplane._v)
    {
    }

    ObjectPlane::~ObjectPlane()
    {
    }

    ObjectPlane& ObjectPlane::operator=(const ObjectPlane& objplane) {
        Plane::operator=(objplane);
        this->_center = objplane._center;
        this->_u = objplane._u;
        this->_v = objplane._v;
        return *this;
    }


    // ------------------------------------------------------------
    // Camera
    // ------------------------------------------------------------

    Camera::Camera()
        : _width(0)
        , _height(0)
        , _sensor()
        , _lens()
        , _objplane()
    {
    }

    Camera::Camera(int imageWidth, int imageHeight, const Vector3& sensorCenter, const Vector3& sensorDir,  const Vector3& sensorUp, double sensorSize,
                   double distSensorToLens, double focalLength, double lensRadius, double sensorSensitivity)
        : _width(imageWidth)
        , _height(imageHeight)
        , _sensor()
        , _lens()
        , _objplane()
    {
        _sensor._center = sensorCenter;
        _sensor._direction = sensorDir;
        _sensor._up = sensorUp;

        _sensor._width = sensorSize * imageWidth / imageHeight;
        _sensor._height = sensorSize;
        
        _sensor._pixelWidth = _sensor._width / imageWidth;
        _sensor._pixelHeight = _sensor._height / imageHeight;

        _sensor._u = sensorDir.cross(sensorUp).normalize() * _sensor._width;
        _sensor._v = _sensor._u.cross(sensorDir).normalize() * _sensor._height;

        Vector3 objplaneCenter = sensorCenter + (focalLength + distSensorToLens) * sensorDir;
        _objplane = ObjectPlane(objplaneCenter, _sensor._u, _sensor._v);

        _lens._radius = lensRadius;
        _lens._center = sensorCenter + distSensorToLens * sensorDir;
        _lens._u = lensRadius * _sensor._u.normalize();
        _lens._v = lensRadius * _sensor._v.normalize();

        _sensor._sensitivity = sensorSensitivity / (_sensor._pixelWidth * _sensor._pixelHeight);
    }

    Camera::Camera(const Camera& camera)
        : _width(camera._width)
        , _height(camera._height)
        , _sensor(camera._sensor)
        , _lens(camera._lens)
        , _objplane(camera._objplane)
    {
    }

    Camera::~Camera()
    {
    }

    Camera& Camera::operator=(const Camera& camera) {
        this->_width = camera._width;
        this->_height = camera._height;
        this->_sensor = camera._sensor;
        this->_lens = camera._lens;
        this->_objplane = camera._objplane;
        return *this;
    }

    bool Camera::intersectLens(const Ray& ray, Vector3& positionOnLens, Vector3& positionOnObjplane, Vector3& positionOnSensor, double& distance) {
        return false;    
    }
}

