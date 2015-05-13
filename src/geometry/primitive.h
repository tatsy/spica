#ifndef SPICA_PRIMITIVE_H_
#define SPICA_PRIMITIVE_H_

#if defined(_WIN32) || defined(__WIN32__)
#define SPICA_PRIMITIVE_DLL __declspec(dllexport)
#else
#define SPICA_PRIMITIVE_DLLT
#endif

#include "../renderer/ray.h"

namespace spica {
    // --------------------------------------------------
    // Interface class for geometries
    // --------------------------------------------------
    class SPICA_PRIMITIVE_DLL Primitive {    
    public:
        virtual ~Primitive() {}
        virtual bool intersect(const Ray& ray, Hitpoint* hitpoint) const = 0;
        virtual double area() const = 0;
    };
}

#endif  // SPICA_PRIMITIVE_H_
