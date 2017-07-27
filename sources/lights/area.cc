#define SPICA_API_EXPORT
#include "area.h"

#include "core/shape.h"
#include "core/visibility_tester.h"
#include "core/sampling.h"
#include "core/interaction.h"

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

    Spectrum AreaLight::L(const Interaction& pLight, const Vector3d& w) const {
        return vect::dot(pLight.normal(), w) > 0.0 ? Lemit_ : Spectrum(0.0);                
    }

    Spectrum AreaLight::sampleLi(const Interaction& pObj, const Point2d& rands,
                                 Vector3d* dir, double* pdf,
                                 VisibilityTester* vis) const {
        Interaction pLight = shape_->sample(pObj, rands);
        *dir = (pLight.pos() - pObj.pos()).normalized();
        *pdf = shape_->pdf(pObj, *dir);
        *vis = VisibilityTester(pObj, pLight);
        return L(pLight, -(*dir));
    }

    double AreaLight::pdfLi(const Interaction& pObj, const Vector3d& dir) const {
        return shape_->pdf(pObj, dir);
    }

    Spectrum AreaLight::sampleLe(const Point2d& randPos, const Point2d& randDir,
                                 Ray* ray, Normal3d* nLight, double* pdfPos,
                                 double* pdfDir) const {
        Interaction pShape = shape_->sample(randPos);
        *pdfPos = shape_->pdf(pShape);
        *nLight = pShape.normal();

        Vector3d w = sampleCosineHemisphere(randDir);
        *pdfDir = cosineHemispherePdf(vect::cosTheta(w));

        Vector3d v1, v2, n(pShape.normal());
        vect::coordinateSystem(n, &v1, &v2);
        w = w.x() * v1 + w.y() * v2 + w.z() * n;
        *ray = pShape.spawnRay(w);
        return L(pShape, w);
    }

    void AreaLight::pdfLe(const Ray& ray, const Normal3d& nLight,
                          double* pdfPos, double* pdfDir) const {
        Interaction it(ray.org(), nLight, Vector3d(nLight));
        *pdfPos = shape_->pdf(it);
        *pdfDir = cosineHemispherePdf(vect::dot(nLight, ray.dir()));
    }

    Spectrum AreaLight::power() const {
        return Lemit_ * area() * PI;
    }

    Light* AreaLight::clone() const {
        return new AreaLight(shape_, lightToWorld_, Lemit_, numSamples_);
    }

}  // namespace spica
