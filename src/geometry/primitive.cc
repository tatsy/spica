#define SPICA_PRIMITIVE_EXPORT
#include "primitive.h"

namespace spica {

    Primitive::Primitive()
        : _material()
    {
    }

    Primitive::Primitive(const Material& material)
        : _material(material)
    {
    }

    Primitive::Primitive(const Primitive& primitive)
        : _material(primitive._material)
    {
    }

    Primitive::~Primitive() 
    {
    }

    Primitive& Primitive::operator=(const Primitive& primitive) {
        this->_material = primitive._material;
        return *this;
    }

    bool Primitive::intersect(const Ray& ray, HitPoint& hitpoint) const {
        msg_assert(false, "Please overload the function \"intersect\"");
    }
}
