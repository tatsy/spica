#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_SPHERE_H_
#define SPICA_SPHERE_H_

#include "core/vector3d.h"
#include "core/ray.h"

#include "render/shape.h"

namespace spica {

/** Sphere class
 *  @ingroup shape_module
 */
class SPICA_EXPORTS Sphere : public Shape {
public:
    Sphere();
    Sphere(const Point3d& center, double radius,
           const Transform& objectToWorld = Transform());
    Sphere(const Sphere& sphere);
    ~Sphere();

    Sphere& operator=(const Sphere& sphere);

    bool intersect(const Ray& ray, double* tHit,
                   SurfaceInteraction* isect) const override;
    bool intersect(const Ray& ray) const override;

    Interaction sample(const Point2d& rands) const override;
    Interaction sample(const Interaction& isect,
                       const Point2d& rands) const override;

    double pdf(const Interaction& pObj, const Vector3d& wi) const override;

    Bounds3d objectBound() const override;

    double area() const override;

    std::vector<Triangle> triangulate() const override;

    inline Point3d  center() const { return center_; }
    inline double radius() const { return radius_; }

private:
    Point3d  center_ = { 0.0, 0.0, 0.0 };
    double radius_ = 0.0;

};  // class Sphere

}  // namespace spica

#endif  // SPICA_SPHERE_H_
