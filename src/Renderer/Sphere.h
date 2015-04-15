#ifndef RAINY_SPHERE_H_
#define RAINY_SPHERE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef RAINY_SPHERE_EXPORT
        #define RAINY_SPHERE_DLL __declspec(dllexport)
    #else
        #define RAINY_SPHERE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define RAINY_SPHERE_DLL
#endif

#include "Primitive.h"
#include "Vector3.h"
#include "material.h"
#include "Ray.h"

namespace rainy {

    class RAINY_SPHERE_DLL Sphere : public Primitive {
    private:
        double _radius;
        Vector3 _center;

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

        inline double radius() const { return _radius; }
        inline Vector3 center() const { return _center; }

    };  // class Sphere

}  // namespace rainy

#endif  // RAINY_SPHERE_H_
