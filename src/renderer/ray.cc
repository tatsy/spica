#define SPICA_RAY_EXPORT
#include "ray.h"
#include "../utils/common.h"

namespace spica {

    Ray::Ray()
        : _origin()
        , _direction() {
    }

    Ray::Ray(const Vector3& origin, const Vector3& direction)
        : _origin(origin)
        , _direction(direction) {
    }

    Ray::Ray(const Ray& ray)
        : _origin(ray._origin)
        , _direction(ray._direction) {
    }

    Ray::~Ray() {
    }

    Ray& Ray::operator=(const Ray& ray) {
        this->_origin = ray._origin;
        this->_direction = ray._direction;
        return *this;
    }

    Hitpoint::Hitpoint()
        : _distance(INFTY)
        , _normal()
        , _position() {
    }

    Hitpoint::Hitpoint(const Hitpoint& hp)
        : _distance(hp._distance)
        , _normal(hp._normal)
        , _position(hp._position) {
    }

    Hitpoint::~Hitpoint() {
    }

    Hitpoint& Hitpoint::operator=(const Hitpoint& hp) {
        this->_distance = hp._distance;
        this->_normal = hp._normal;
        this->_position = hp._position;
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
