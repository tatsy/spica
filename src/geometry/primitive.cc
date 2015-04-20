#define SPICA_PRIMITIVE_EXPORT
#include "primitive.h"

namespace spica {

    Primitive::Primitive()
        : _emission()
        , _color()
        , _reftype()
    {
    }

    Primitive::Primitive(const Color& emission, const Color& color, ReflectionType reftype)
        : _emission(emission)
        , _color(color)
        , _reftype(reftype)
    {
    }

    Primitive::Primitive(const Primitive& primitive)
        : _emission(primitive._emission)
        , _color(primitive._color)
        , _reftype(primitive._reftype)
    {
    }

    Primitive::~Primitive() 
    {
    }

    Primitive& Primitive::operator=(const Primitive& primitive) {
        this->_emission = primitive._emission;
        this->_color = primitive._color;
        this->_reftype = primitive._reftype;
        return *this;
    }

    bool Primitive::intersect(const Ray& ray, HitPoint& hitpoint) const {
        msg_assert(false, "Please overload the function \"intersect\"");
    }
}
