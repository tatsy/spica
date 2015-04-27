#ifndef SPICA_MATERIAL_H_
#define SPICA_MATERIAL_H_

#include "../utils/color.h"

namespace spica {

    enum ReflectionType {
        REFLECTION_DIFFUSE    = 0x01,
        REFLECTION_SPECULAR   = 0x02,
        REFLECTION_REFRACTION = 0x04,
        REFLECTION_SUBSURFACE = 0x08,
    };

    const double IOR_VACCUM = 1.0;
    const double IOR_OBJECT = 1.5;
    const double REFLECT_PROBABLITY = 0.5;

    struct Material {
        Color emission;
        Color color;
        ReflectionType reftype;
        Material()
            : emission()
            , color()
            , reftype(REFLECTION_DIFFUSE)
        {
        }

        Material(const Color& emission_, const Color& color_, const ReflectionType reftype_)
            : emission(emission_)
            , color(color_)
            , reftype(reftype_)
        {
        }
    };
}

#endif
