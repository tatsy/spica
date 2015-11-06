#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_AREA_LIGHT_H_
#define _SPICA_AREA_LIGHT_H_

#include <vector>

#include "light_interface.h"

#include "../core/image.h"
#include "../shape/triangle.h"

namespace spica {

    /** Area light class
     *  @ingroup light_module
     */
    class SPICA_EXPORTS AreaLight : public ILight {
    private:
        Color _emittance;
        std::vector<Triangle> _triangles;
        std::vector<double>   _samplePdf;
        double _totalArea;

    public:
        AreaLight();
        AreaLight(const Trimesh& triangles, const Color& emittance);
        AreaLight(const AreaLight& l);
        AreaLight(AreaLight&& l);

        ~AreaLight();

        AreaLight& operator=(const AreaLight& l);
        AreaLight& operator=(AreaLight&& l);

        LightSample sample(double r1, double r2, double r3) const override;
        Color directLight(const Vector3D& dir) const override;
		Color globalLight(const Vector3D& dir) const override;
        double area() const override;
        ILight* clone() const override;

    private:
        void calcSamplePdf();
    };

}  // namespace spica

#endif  // _SPICA_AREA_LIGHT_H_