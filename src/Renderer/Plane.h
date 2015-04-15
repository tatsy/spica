#ifndef RAINY_PLANE_H_
#define RAINY_PLANE_H_

#if defined(_WINM32) || defined(__WIN32__)
    #ifdef RAINY_PLANE_EXPORT
        #define RAINY_PLANE_DLL __declspec(dllexport)
    #else
        #define RAINY_PLANE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define RAINY_PLANE_DLL
#endif

#include "Primitive.h"

namespace rainy {

    class Plane : public Primitive {
    private:
        double _distance;
        Vector3 _normal;

    public:
        Plane();
        Plane(double distance, const Vector3& normal, const Color& emission, const Color& color, ReflectionType reftype);
        Plane(const Plane& plane);
        ~Plane();

        Plane& operator=(const Plane& plane);

        bool intersect(const Ray& ray, HitPoint& hitpoint) const;
    };
}

#endif
