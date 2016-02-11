#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ENVMAP_H_
#define _SPICA_ENVMAP_H_

#include <vector>

#include "light.h"

#include "../core/spectrum.h"
#include "../image/mipmap.h"
#include "../math/vector3d.h"
#include "../shape/sphere.h"
#include "../core/sampling.h"

namespace spica {

    class Photon;

    /** Environment mapping
     *  @ingroup light_module
     */
    class SPICA_EXPORTS Envmap : public Light {
    public:
        /** The Envmap constructor. */
        Envmap(const Image& texmap, const Transform& lightToWorld,
               const Spectrum& L, int numSamples);

        virtual ~Envmap();

        Spectrum sampleLi(const Interaction& pObj, const Point2D& rands,
                          Vector3D* dir, double* pdf, VisibilityTester* vis) const override;

        double pdfLi(const Interaction& pObj, const Vector3D& dir) const override;

        Spectrum Le(const Ray& ray) const override;
        Spectrum power() const override;

        Light* clone() const override;

    private:
        const MipMap texmap_;
        Vector3D worldCenter_;
        double   worldRadius_;
        Distribution2D distrib_;
    };

}  // namespace spica

#endif  // _SPICA_ENVMAP_H_
