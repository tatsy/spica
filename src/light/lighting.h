#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_LIGHTING_H_
#define _SPICA_LIGHTING_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_LIGHTING_EXPORT
        #define SPICA_LIGHTING_DLL __declspec(dllexport)
    #else
        #define SPICA_LIGHTING_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_LIGHTING_DLL
#endif

#include <memory>
#include <string>

#include "light_interface.h"

#include "../utils/vector3d.h"
#include "../utils/stack.h"
#include "../utils/image.h"
#include "../geometry/triangle.h"
#include "../geometry/sphere.h"

namespace spica {

    enum class LightType {
        AreaLight = 0x01,
        Envmap    = 0x02
    };

    class SPICA_LIGHTING_DLL Lighting {
    private:
        ILight* _ptr;

    public:
        //! Constructor
        Lighting();
        Lighting(const Lighting& l);
        Lighting(Lighting&& l);

        //! Destructor
        ~Lighting();

        Lighting& operator=(const Lighting& l);
        Lighting& operator=(Lighting&& l);

        //! Initialize lighting as environment map (with image file name)
        static Lighting asEnvmap(const Sphere& boundSphere, const std::string& filename);

        //! Initialize lighting as environment map (with image)
        static Lighting asEnvmap(const Sphere& boundSphere, const Image& image);

        //! Initialize lighting as area light
        static Lighting asAreaLight(const std::vector<Triangle>& triangles, const Color& emittance);

        LightSample sample(Stack<double>& rstack) const;
        Color directLight(const Vector3D& dir) const;
        double area() const;
    };

}  // namespace spica

#endif  // _SPICA_LIGHTING_H_
