#define SPICA_API_EXPORT
#include "area_light.h"

#include "../shape/shape.h"
#include "../shape/visibility_tester.h"
#include "../core/sampling.h"
#include "../core/interaction.h"

namespace spica {

    AreaLight::AreaLight(const std::shared_ptr<Shape>& shape,
                         const Transform& lightToWorld,
                         const Spectrum& Lemit,
                         int numSamples)
        : Light{ LightType::Area, lightToWorld, numSamples }
        , shape_{ shape }
        , Lemit_{ Lemit } {
    }

    AreaLight::~AreaLight() {
    }

    Spectrum AreaLight::L(const Interaction& pLight, const Vector3D& w) const {
        return vect::dot(pLight.normal(), w) > 0.0 ? Lemit_ : Spectrum(0.0);                
    }

    Spectrum AreaLight::sampleLi(const Interaction& pObj, const Point2D& rands,
                                 Vector3D* dir, double* pdf,
                                 VisibilityTester* vis) const {
        Interaction pLight = shape_->sample(pObj, rands);
        *dir = (pLight.pos() - pObj.pos()).normalized();
        *pdf = shape_->pdf(pObj, *dir);
        *vis = VisibilityTester(pObj, pLight);
        return L(pLight, -(*dir));
    }

    double AreaLight::pdfLi(const Interaction& pObj, const Vector3D& dir) const {
        return shape_->pdf(pObj, dir);
    }

    Spectrum AreaLight::sampleLe(const Point2D& randPos, const Point2D& randDir,
                                 Ray* ray, Normal3D* nLight, double* pdfPos,
                                 double* pdfDir) const {
        Interaction pShape = shape_->sample(randPos);
        *pdfPos = shape_->pdf(pShape);
        *nLight = pShape.normal();

        Vector3D w = sampleCosineHemisphere(randDir);
        *pdfDir = cosineHemispherePdf(w.z());

        Vector3D v1, v2, n(pShape.normal());
        vect::coordinateSystem(n, &v1, &v2);
        w = w.x() * v1 + w.y() * v2 + w.z() * n;
        *ray = pShape.spawnRay(w);
        return L(pShape, w);
    }

    void AreaLight::pdfLe(const Ray& ray, const Normal3D& nLight,
                          double* pdfPos, double* pdfDir) const {
        
    }

    Spectrum AreaLight::power() const {
        return Lemit_ * area() * PI;
    }

    Light* AreaLight::clone() const {
        return new AreaLight(shape_, lightToWorld_, Lemit_, numSamples_);
    }

}  // namespace spica
