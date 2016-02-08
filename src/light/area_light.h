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

        LightSample sample(const Vector3D& v, Stack<double>& rands) const override;
        Photon samplePhoton(Stack<double>& rands) const override;
        Color directLight(const Vector3D& dir) const override;
        Color globalLight(const Vector3D& dir) const override;
        double area() const override;
        Light* clone() const override;

    private:
        void calcSamplePdf();
        void sampleOnLight(Vector3D* pos, Vector3D* nrm, Stack<double>& rands) const;
    };

}  // namespace spica

#endif  // _SPICA_AREA_LIGHT_H_