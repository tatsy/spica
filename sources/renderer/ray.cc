#define SPICA_API_EXPORT
#include "ray.h"
#include "../core/common.h"

#include <cmath>

namespace spica {

    Ray::Ray() {
    }

    Ray::Ray(const Point& origin, const Vector3D& direction)
        : origin_{ origin }
        , direction_{ direction }
        , invdir_{} {
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
        this->origin_    = ray.origin_;
        this->direction_ = ray.direction_;
        this->invdir_    = ray.invdir_;
        return *this;
    }

    void Ray::calcInvdir() {
        invdir_.xRef() = (direction_.x() == 0.0) ? INFTY : 1.0 / direction_.x();
        invdir_.yRef() = (direction_.y() == 0.0) ? INFTY : 1.0 / direction_.y();
        invdir_.zRef() = (direction_.z() == 0.0) ? INFTY : 1.0 / direction_.z();
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

    Intersection::Intersection(int objectID, const Hitpoint& hp, const Spectrum& color)
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
