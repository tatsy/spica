#include "Sphere.h"

namespace rainy {

    Sphere::Sphere()
        : _radius(0.0)
        , _center()
        , _emission()
        , _color()
        , _reftype(REFLECTION_DIFFUSE) {
    }

    Sphere::Sphere(double radius, const Vector3& center, const Color& emission, const Color& color, ReflectionType reftype)
        : _radius(radius)
        , _center(center)
        , _emission(emission)
        , _color(color)
        , _reftype(reftype) {
    }

    Sphere::Sphere(const Sphere& sphere)
        : _radius(sphere._radius)
        , _center(sphere._center)
        , _emission(sphere._emission)
        , _color(sphere._color)
        , _reftype(sphere._reftype) {
    }

    Sphere::~Sphere() {
    }

    Sphere& Sphere::operator=(const Sphere& sphere) {
        this->_radius = sphere._radius;
        this->_center = sphere._center;
        this->_emission = sphere._emission;
        this->_color = sphere._color;
        this->_reftype = sphere._reftype;
        return *this;
    }

}  // namespace rainy
