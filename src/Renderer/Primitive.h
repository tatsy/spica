#ifndef RAINY_PRIMITIVE_H_
#define RAINY_PRIMITIVE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef RAINY_PRIMITIVE_EXPORT
        #define RAINY_PRIMITIVE_DLL __declspec(dllexport)
    #else
        #define RAINY_PRIMITIVE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define RAINY_PRIMITIVE_DLL
#endif

#include "Ray.h"
#include "material.h"

namespace rainy {
    /* Interface class for shape primitives (Plane, Sphere etc.)
     */
    class RAINY_PRIMITIVE_DLL Primitive {
    protected:
        Color _emission;
        Color _color;
        ReflectionType _reftype;

    public:
        Primitive();
        Primitive(const Color& emission, const Color& color, ReflectionType reftype);
        Primitive(const Primitive& primitive);
        virtual ~Primitive();

        Primitive& operator=(const Primitive& primitive);

        virtual bool intersect(const Ray& ray, HitPoint& hitpoint) const = 0;

        inline Color emission() const { return _emission; }
        inline Color color()    const { return _color;    }
        inline ReflectionType reftype() const { return _reftype; }
    };
}

#endif  // RAINY_PRIMITIVE_H_
