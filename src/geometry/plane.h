#ifndef SPICA_PLANE_H_
#define SPICA_PLANE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_PLANE_EXPORT
        #define SPICA_PLANE_DLL __declspec(dllexport)
    #else
        #define SPICA_PLANE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_PLANE_DLL
#endif

#include "primitive.h"

namespace spica {

    class SPICA_PLANE_DLL Plane : public Primitive {
    protected:
        double _distance;
        Vector3 _normal;

    public:
        Plane();
        Plane(double distance, const Vector3& normal, const Color& emission, const Color& color, ReflectionType reftype);
        Plane(const Plane& plane);
        virtual ~Plane();

        Plane& operator=(const Plane& plane);

        bool intersect(const Ray& ray, HitPoint& hitpoint) const;

        inline double distance() const { return _distance; }
        inline Vector3 normal() const { return _normal; }
    };
}

#endif  // SPICA_PLANE_H_
