#ifndef SPICA_PRIMITIVE_H_
#define SPICA_PRIMITIVE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_PRIMITIVE_EXPORT
        #define SPICA_PRIMITIVE_DLL __declspec(dllexport)
    #else
        #define SPICA_PRIMITIVE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_PRIMITIVE_DLL
#endif

#include "../utils/common.h"
#include "../renderer/Ray.h"
#include "../renderer/material.h"

namespace spica {
    /* Interface class for shape primitives (Plane, Sphere etc.)
     */
    class SPICA_PRIMITIVE_DLL Primitive {
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

        virtual bool intersect(const Ray& ray, HitPoint& hitpoint) const;

        inline Color emission() const { return _emission; }
        inline Color color()    const { return _color;    }
        inline ReflectionType reftype() const { return _reftype; }
    };
}

#endif  // SPICA_PRIMITIVE_H_
