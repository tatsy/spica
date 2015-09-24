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

#include <vector>

#include "light_interface.h"

#include "../utils/image.h"
#include "../geometry/triangle.h"

namespace spica {

    class SPICA_AREA_LIGHT_DLL AreaLight : public ILight {
    private:
        Color _emittance;
        std::vector<Triangle> _triangles;
        std::vector<double>   _samplePdf;
        double _totalArea;

    public:
        AreaLight();
        AreaLight(const std::vector<Triangle>& triangles, const Color& emittance);
        AreaLight(const AreaLight& l);
        AreaLight(AreaLight&& l);

        ~AreaLight();

        AreaLight& operator=(const AreaLight& l);
        AreaLight& operator=(AreaLight&& l);

        LightSample sample(double r1, double r2, double r3) const override;
        Color directLight(const Vector3D& dir) const override;
        double area() const override;
        ILight* clone() const override;

    private:
        void calcSamplePdf();
    };

}  // namespace spica

#endif  // _SPICA_AREA_LIGHT_H_