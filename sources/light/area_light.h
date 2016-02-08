#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_AREA_LIGHT_H_
#define _SPICA_AREA_LIGHT_H_

#include <vector>

#include "light_interface.h"

#include "../shape/triangle.h"

namespace spica {

    /** Area light class.
     *  @ingroup light_module
     */
    class SPICA_EXPORTS AreaLight : public Light {
    private:
        Spectrum _emittance;
        std::vector<Triangle> _triangles;
        std::vector<double>   _samplePdf;
        double _totalArea;

    public:
        AreaLight();
        AreaLight(const Trimesh& triangles, const Spectrum& emittance);
        AreaLight(const AreaLight& l);
        AreaLight(AreaLight&& l);

        ~AreaLight();

        AreaLight& operator=(const AreaLight& l);
        AreaLight& operator=(AreaLight&& l);

        LightSample sample(const Point& v, Stack<double>& rands) const override;
        Photon samplePhoton(Stack<double>& rands) const override;
        Spectrum directLight(const Vector3D& dir) const override;
        Spectrum globalLight(const Vector3D& dir) const override;
        double area() const override;
        Light* clone() const override;

    private:
        void calcSamplePdf();
        void sampleOnLight(Point* pos, Normal* nrm, Stack<double>& rands) const;
    };

}  // namespace spica

#endif  // _SPICA_AREA_LIGHT_H_