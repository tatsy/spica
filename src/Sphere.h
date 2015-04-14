#ifndef RAINY_SPHERE_H_
#define RAINY_SPHERE_H_

#include "Vector3.h"
#include "material.h"
#include "Ray.h"

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

        /* Compute intersection with ray
         * @param[in] ray: a ray casted to the sphere
         * @param[out] hitpoint: if ray hits to the sphere, intersecting point is inserted
         * @param[return] boolean to represent if ray hits the sphere or not
         */
        bool intersect(const Ray& ray, HitPoint& hitpoint) const;

        inline Color color() const { return _color; }
        inline Color emission() const { return _emission; }
        inline ReflectionType reftype() const { return _reftype; }

    };  // class Sphere

}  // namespace rainy

#endif  // RAINY_SPHERE_H_
