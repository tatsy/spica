#ifndef SPICA_MATERIAL_H_
#define SPICA_MATERIAL_H_

#include "../utils/Color.h"

namespace spica {

    enum ReflectionType {
        REFLECTION_DIFFUSE    = 0x01,
        REFLECTION_SPECULAR   = 0x02,
        REFLECTION_REFRACTION = 0x04,
        REFLECTION_SUBSURFACE = 0x08,
    };

    const double indexOfRef = 1.5;
}

#endif
