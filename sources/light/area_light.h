#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_AREA_LIGHT_H_
#define _SPICA_AREA_LIGHT_H_

#include <vector>
#include <memory>

#include "light.h"
#include "../shape/shape.h"

namespace spica {

    /** Area light class.
     *  @ingroup light_module
     */
    class SPICA_EXPORTS AreaLight : public Light {
    public:
        AreaLight(const std::shared_ptr<Shape>& shape,
                  const Transform& lightToWorld,
                  const Spectrum& Lemit,
                  int numSamples = 1);

        virtual ~AreaLight();

        Spectrum L(const Interaction& pLight, const Vector3d& dir) const;

        Spectrum sampleLi(const Interaction& isect, const Point2d& rands,
                          Vector3d* dir, double* pdf,
                          VisibilityTester* vis) const override;

        double pdfLi(const Interaction& pObj, const Vector3d& dir) const override;

        Spectrum sampleLe(const Point2d& rand1, const Point2d& rand2,
                          Ray* ray, Normal3d* nLight, double* pdfPos,
                          double* pdfDir) const override;
        void pdfLe(const Ray& ray, const Normal3d& nLight, double* pdfPos,
                   double* pdfDir) const override;

        Spectrum power() const override;
        Light* clone() const override;

        inline double area() const {
            return shape_->area();
        }

    protected:
        std::shared_ptr<Shape> shape_;
        const Spectrum Lemit_;
    };

}  // namespace spica

#endif  // _SPICA_AREA_LIGHT_H_