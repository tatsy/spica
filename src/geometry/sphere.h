#ifndef SPICA_SPHERE_H_
#define SPICA_SPHERE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SPHERE_EXPORT
        #define SPICA_SPHERE_DLL __declspec(dllexport)
    #else
        #define SPICA_SPHERE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_SPHERE_DLL
#endif

#include "primitive.h"
#include "../utils/vector3.h"
#include "../renderer/material.h"
#include "../renderer/ray.h"

namespace spica {

    class SPICA_SPHERE_DLL Sphere : public Primitive {
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

}  // namespace spica

#endif  // SPICA_SPHERE_H_
