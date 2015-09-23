#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_AREA_LIGHT_H_
#define _SPICA_AREA_LIGHT_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_AREA_LIGHT_EXPORT
        #define SPICA_AREA_LIGHT_DLL __declspec(dllexport)
    #else
        #define SPICA_AREA_LIGHT_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_AREA_LIGHT_DLL
#endif

#include "light_interface.h"

#include "../utils/image.h"
#include "../geometry/triangle.h"

namespace spica {

    class AreaLight : public ILight {
    public:
        virtual Color L(const Vector3D& pos, const Vector3D& normal, const Vector3D& w) const = 0;
    };

    class DiffuseAreaLight : public AreaLight {
    private:
        Color _emittance;
        Triangle _triangle;

    public:
        DiffuseAreaLight(const Triangle& tri, const Color& emittance);
        ~DiffuseAreaLight();

        Color L(const Vector3D& pos, const Vector3D& normal, const Vector3D& w) const override;
    };

    class InfiniteAreaLight : public AreaLight {
    private:
        const Color& _lightness;
        const Image& _envmap;

    public:
        InfiniteAreaLight(const Color& lightness, const Image& envmap);
        ~InfiniteAreaLight();

        Color L(const Vector3D& pos, const Vector3D& normal, const Vector3D& w) const override;

    private:
        void computeSamplePdf();
    };

}  // namespace spica

#endif  // _SPICA_AREA_LIGHT_H_