#define SPICA_API_EXPORT
#include "ray.h"
#include "../core/common.h"

#include <cmath>

namespace spica {

    Ray::Ray() {
    }

    Ray::Ray(const Vector3D& origin, const Vector3D& direction)
        : _origin{origin}
        , _direction{direction}
        , _invdir{} {
        Assertion(std::abs(1.0 - direction.norm()) < EPS,
                  "Direction must be unit vector!!");
        calcInvdir();
    }

    Ray::Ray(const Ray& ray)
        : Ray {} {
        operator=(ray);
    }

    Ray::~Ray() {
    }

    Ray& Ray::operator=(const Ray& ray) {
        this->_origin    = ray._origin;
        this->_direction = ray._direction;
        this->_invdir    = ray._invdir;
        return *this;
    }

    void Ray::calcInvdir() {
        _invdir.xRef() = (_direction.x() == 0.0) ? INFTY : 1.0 / _direction.x();
        _invdir.yRef() = (_direction.y() == 0.0) ? INFTY : 1.0 / _direction.y();
        _invdir.zRef() = (_direction.z() == 0.0) ? INFTY : 1.0 / _direction.z();
    }

    Hitpoint::Hitpoint() {
    }

    Hitpoint::Hitpoint(const Hitpoint& hp) 
        : Hitpoint{} {
        this->operator=(hp);
    }

    Hitpoint::~Hitpoint() {
    }

    Hitpoint& Hitpoint::operator=(const Hitpoint& hp) {
        this->_distance = hp._distance;
        this->_position = hp._position;
        this->_normal   = hp._normal;
        this->_texcoord = hp._texcoord;
        return *this;
    }

    Intersection::Intersection() {
    }

    Intersection::Intersection(const Intersection& isect)
        : Intersection{} {
        this->operator=(isect);
    }

    Intersection::Intersection(int objectID, const Hitpoint& hp, const Color& color)
        : _distance{hp.distance()}
        , _pos{hp.position()}
        , _normal{hp.normal()}
        , _color{color}
        , _objID{objectID} {
    }

    Intersection::~Intersection() {
    }

    Intersection& Intersection::operator=(const Intersection& isect) {
        this->_distance = isect._distance;
        this->_pos      = isect._pos;
        this->_normal   = isect._normal;
        this->_color    = isect._color;
        this->_objID    = isect._objID;
        return *this;
    }

}  // namespace spica
