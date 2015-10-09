#define SPICA_RAY_EXPORT
#include "ray.h"
#include "../utils/common.h"

#include <cmath>

namespace spica {

    Ray::Ray()
        : _origin()
        , _direction()
        , _invdir() {
        calcInvdir();
    }

    Ray::Ray(const Vector3D& origin, const Vector3D& direction)
        : _origin(origin)
        , _direction(direction)
        , _invdir() {
        Assertion(std::abs(1.0 - direction.norm()) < EPS, "Direction must be unit vector");
        calcInvdir();
    }

    Ray::Ray(const Ray& ray)
        : _origin()
        , _direction()
        , _invdir() {
        operator=(ray);
    }

    Ray::~Ray() {
    }

    Ray& Ray::operator=(const Ray& ray) {
        this->_origin = ray._origin;
        this->_direction = ray._direction;
        this->_invdir = ray._invdir;
        return *this;
    }

    void Ray::calcInvdir() {
        _invdir.x() = (_direction.x() == 0.0) ? INFTY : 1.0 / _direction.x();
        _invdir.y() = (_direction.y() == 0.0) ? INFTY : 1.0 / _direction.y();
        _invdir.z() = (_direction.z() == 0.0) ? INFTY : 1.0 / _direction.z();
    }

    Hitpoint::Hitpoint()
        : _distance(INFTY)
        , _normal()
        , _position()
        , _texcoord(INFTY, INFTY) {
    }

    Hitpoint::Hitpoint(const Hitpoint& hp)
        : _distance(hp._distance)
        , _normal(hp._normal)
        , _position(hp._position)
        , _texcoord(hp._texcoord) {
    }

    Hitpoint::~Hitpoint() {
    }

    Hitpoint& Hitpoint::operator=(const Hitpoint& hp) {
        this->_distance = hp._distance;
        this->_normal   = hp._normal;
        this->_position = hp._position;
        this->_texcoord = hp._texcoord;
        return *this;
    }

    Intersection::Intersection()
        : _hitPoint()
        , _objectId(-1) {
    }

    Intersection::Intersection(const Intersection& intersection)
        : _hitPoint(intersection._hitPoint)
        , _objectId(intersection._objectId) {
    }

    Intersection::~Intersection() {
    }

    Intersection& Intersection::operator=(const Intersection& intersection) {
        this->_hitPoint = intersection._hitPoint;
        this->_objectId = intersection._objectId;
        return *this;
    }

    double Intersection::hittingDistance() const {
        return _hitPoint.distance();
    }

}  // namespace spica
