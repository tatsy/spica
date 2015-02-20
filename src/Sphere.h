#ifndef RAINY_SPHERE_H_
#define RAINY_SPHERE_H_

#include "Vector3.h"
#include "material.h"

namespace rainy {

    class Sphere {
    private:
        double _radius;
        Vector3 _center;
        Color _emission;
        Color _color;
        ReflectionType _reftype;

    public:
        Sphere();
        Sphere(double radius, const Vector3& center, const Color& emission, const Color& color, ReflectionType reftype);
        Sphere(const Sphere& sphere);
        ~Sphere();

        Sphere& operator=(const Sphere& sphere);

    };  // class Sphere

}  // namespace rainy

#endif  // RAINY_SPHERE_H_
