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
        , _distSensorToLens(0.0)
        , _sensor()
        , _lens()
        , _objplane()
    {
    }

    Camera::Camera(int imageWidth, int imageHeight, const Vector3& sensorCenter, const Vector3& sensorDir,  const Vector3& sensorUp, double sensorSize,
                   double distSensorToLens, double focalLength, double lensRadius, double sensorSensitivity)
        : _width(imageWidth)
        , _height(imageHeight)
        , _distSensorToLens(distSensorToLens)
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
        _lens._focalLength = focalLength;
        _lens._center = sensorCenter + distSensorToLens * sensorDir;
        _lens._u = lensRadius * _sensor._u.normalize();
        _lens._v = lensRadius * _sensor._v.normalize();
        _lens._normal = sensorDir.normalize();

        _sensor._sensitivity = sensorSensitivity / (_sensor._pixelWidth * _sensor._pixelHeight);
    }

    Camera::Camera(const Camera& camera)
        : _width(camera._width)
        , _height(camera._height)
        , _distSensorToLens(camera._distSensorToLens)
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
        this->_distSensorToLens = camera._distSensorToLens;
        this->_sensor = camera._sensor;
        this->_lens = camera._lens;
        this->_objplane = camera._objplane;
        return *this;
    }

    double Camera::PImageToPAx1(const double PImage, const Vector3& x0xV, const Vector3& x0x1, const Vector3& orientNormal) const {
        double ratio = _distSensorToLens / _lens._focalLength;
        double lengthRatio = x0xV.dot(x0xV) / x0x1.dot(x0x1);
        double dirRatio = (-1.0 * x0x1).normalize().dot(orientNormal) / x0x1.normalize().dot(_sensor._direction);
        return ratio * ratio * lengthRatio * dirRatio;
    }

    double Camera::samplingPdfOnLens() const {
        return 1.0 / (PI * _lens._radius * _lens._radius);
    }

    double plane_intersection(const Vector3& normal, const Vector3& pos, const Ray& ray) {
        const double pn = pos.dot(normal);
        const double on = ray.origin().dot(normal);
        const double dn = ray.direction().dot(normal);

        if (abs(dn) > EPS) {
            const double t = (pn - on) / dn;
            return t;
        }
        return -INFTY;
    }

    double Camera::intersectLens(const Ray& ray, Vector3& positionOnLens, Vector3& positionOnObjplane, Vector3& positionOnSensor, Vector3& uvOnSensor) const {
        const Vector3 lensNormal = _sensor._direction.normalize();
        const Vector3 objplaneNormal = lensNormal;

        const double lensT = plane_intersection(lensNormal, _lens._center, ray);
        if (EPS < lensT) {
            positionOnLens = ray.origin() + lensT * ray.direction();
            if ((positionOnLens - _lens._center).norm() < _lens._radius && _lens._normal.dot(ray.direction()) <= 0.0) {
                const double objplaneT = plane_intersection(objplaneNormal, _objplane._center, ray);
                positionOnObjplane = ray.origin() + objplaneT * ray.direction();
                const double uOnObjplane = (positionOnObjplane - _objplane._center).dot(_objplane._u.normalize()) / _objplane._u.norm();
                const double vOnObjplane = (positionOnObjplane - _objplane._center).dot(_objplane._v.normalize()) / _objplane._v.norm();

                const double ratio = _distSensorToLens / _lens._focalLength;
                const double uOnSensor = -ratio * uOnObjplane;
                const double vOnSensor = -ratio * vOnObjplane;
                positionOnSensor = _sensor._center + uOnSensor * _sensor._u + vOnObjplane * _sensor._v;

                if (-0.5 <= uOnSensor && uOnSensor < 0.5 && -0.5 <= vOnSensor && vOnSensor < 0.5) {
                    uvOnSensor.setX((uOnSensor + 0.5) * _width);
                    uvOnSensor.setY((vOnSensor + 0.5) * _height);
                    return lensT;
                }
            }
        }
        return -INFTY;
    }

    double Camera::contribSensitivity(const Vector3& x0xV, const Vector3& x0xI, const Vector3& x0x1) const {
        double lengthRatio = x0xV.dot(x0xV) / x0xI.dot(x0xI);
        double a = _distSensorToLens * (x0xI.normalize().dot(-1.0 * _sensor._direction.normalize()));
        double b = _lens._focalLength * (x0x1.normalize().dot(_sensor._direction.normalize()));
        return _sensor._sensitivity * lengthRatio * pow(a / b, 2.0); 
    }



    void Camera::samplePoints(const int imageX, const int imageY, const Random& rng, Vector3& positionOnSensor, Vector3& positionOnObjplane, Vector3& positionOnLens, double& PImage, double& PLens) const {
        const double uOnPixel = rng.randReal();
        const double vOnPixel = rng.randReal();

        const double uOnSensor = ((imageX + uOnPixel) / this->_width - 0.5);
        const double vOnSensor = ((imageY + vOnPixel) / this->_height - 0.5);
        positionOnSensor = _sensor._center + uOnSensor * _sensor._u + vOnSensor * _sensor._v;

        const double ratio = _lens._focalLength / _distSensorToLens;
        const double uOnObjplane = -ratio * uOnSensor;
        const double vOnObjplane = -ratio * vOnSensor;
        positionOnObjplane = _objplane._center + uOnObjplane * _objplane._u + vOnObjplane * _objplane._v;

        const double r0 = sqrt(rng.randReal());
        const double r1 = rng.randReal() * 2.0 * PI;
        const double uOnLens = r0 * cos(r1);
        const double vOnLens = r0 * sin(r1);
        positionOnLens = _lens._center + uOnLens * _lens._u + vOnLens * _lens._v;

        PImage = 1.0 / (_sensor._pixelWidth * _sensor._pixelHeight);
        PLens = samplingPdfOnLens();
    }

}

