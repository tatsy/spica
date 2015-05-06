#ifndef SPICA_PRIMITIVE_H_
#define SPICA_PRIMITIVE_H_

#include "../renderer/ray.h"

namespace spica {
    /* Interface class for geometries
     */
    class Primitive {    
    public:
        virtual ~Primitive() {}
        virtual bool intersect(const Ray& ray, Hitpoint* hitpoint) const = 0;
        virtual double area() const = 0;
    };
}

#endif  // SPICA_PRIMITIVE_H_
