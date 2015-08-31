#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_LIGHT_INTERFACE_
#define _SPICA_LIGHT_INTERFACE_

#if defined(_WIN32) || defined(__WIN32__)
#define SPICA_LIGHT_INTERFACE __declspec(dllexport)
#else
#define SPICA_LIGHT_INTERFACE
#endif

#include "../utils/vector3d.h"
#include "../utils/color.h"

namespace spica {

    class ILight {
    public:
        ILight() {}
        virtual ~ILight() {}

        virtual Color sampleL(const Vector3D& pos, double pEps, Vector3D* wi, double& pdf) const = 0;

    };

}  // spica

#endif  // _SPICA_LIGHT_INTERFACE_
