#define RAINY_SPHERE_EXPORT
#include "Sphere.h"

#include <cmath>

#include "common.h"

namespace rainy {

    Sphere::Sphere()
        : Primitive()
        , _radius(0.0)
        , _center()
    {
    }

    Sphere::Sphere(double radius, const Vector3& center, const Color& emission, const Color& color, ReflectionType reftype)
        : Primitive(emission, color, reftype)
        , _radius(radius)
        , _center(center)
    {
    }

    Sphere::Sphere(const Sphere& sphere)
        : Primitive(sphere)
        , _radius(sphere._radius)
        , _center(sphere._center)
    {
    }

    Sphere::~Sphere()
    {
    }

    Sphere& Sphere::operator=(const Sphere& sphere) {
        Primitive::operator=(sphere);
        this->_radius = sphere._radius;
        this->_center = sphere._center;
        return *this;
    }

    bool Sphere::intersect(const Ray& ray, HitPoint& hitpoint) const {
        const Vector3 VtoC = _center - ray.origin();
        const double b = VtoC.dot(ray.direction());
        const double D4 = b * b - VtoC.dot(VtoC) + _radius * _radius;

        if (D4 < 0.0) return false;

        const double sqrtD4 = sqrt(D4);
        const double t1 = b - sqrtD4;
        const double t2 = b + sqrtD4;

        if (t1 < EPS && t2 < EPS) return false;

        if (t1 > EPS) {
            hitpoint.setDistance(t1);
        } else {
            hitpoint.setDistance(t2);
        }

        hitpoint.setPosition(ray.origin() + hitpoint.distance() * ray.direction());
        hitpoint.setNormal((hitpoint.position() - _center).normalize());

        return true;
    }

}  // namespace rainy
