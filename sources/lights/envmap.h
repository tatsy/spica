#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ENVMAP_H_
#define _SPICA_ENVMAP_H_

#include <vector>

#include "render/light.h"

#include "core/spectrum.h"
#include "core/vector3d.h"
#include "core/bsphere.h"
#include "core/mipmap.h"
#include "render/sampling.h"

namespace spica {

    class Photon;

    /** Environment mapping
     *  @ingroup light_module
     */
    class SPICA_EXPORTS Envmap : public Light {
    public:
        /** The Envmap constructor. */
        Envmap(const BSphere& worldSphere, const Image& texmap, const Transform& lightToWorld,
               const Spectrum& L, int numSamples);

        virtual ~Envmap();

        Spectrum sampleLi(const Interaction& pObj, const Point2d& rands,
                          Vector3d* dir, double* pdf, VisibilityTester* vis) const override;

        double pdfLi(const Interaction& pObj, const Vector3d& dir) const override;

        Spectrum sampleLe(const Point2d& rand1, const Point2d& rand2,
                          Ray* ray, Normal3d* nLight, double* pdfPos,
                          double* pdfDir) const override;
        void pdfLe(const Ray& ray, const Normal3d& nLight, double* pdfPos,
                   double* pdfDir) const override;

        Spectrum Le(const Ray& ray) const override;
        Spectrum power() const override;

        Light* clone() const override;

    private:
        std::unique_ptr<const MipMap> mipmap_;
        Point3d worldCenter_;
        double   worldRadius_;
        Distribution2D distrib_;
    };

}  // namespace spica

#endif  // _SPICA_ENVMAP_H_
