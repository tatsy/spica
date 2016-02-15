#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_SPHERE_H_
#define SPICA_SPHERE_H_

#include "shape.h"
#include "../math/vector3d.h"
#include "../renderer/ray.h"

namespace spica {

/** Sphere class
 *  @ingroup shape_module
 */
class SPICA_EXPORTS Sphere : public Shape {
public:
    Sphere();
    Sphere(const Point& center, double radius,
           const Transform& objectToWorld = Transform());
    Sphere(const Sphere& sphere);
    ~Sphere();

    Sphere& operator=(const Sphere& sphere);

    bool intersect(const Ray& ray, double* tHit,
                   SurfaceInteraction* isect) const override;

    Bound3d objectBound() const override;

    double area() const override;

    std::vector<Triangle> triangulate() const override;

    inline Point  center() const { return center_; }
    inline double radius() const { return radius_; }

private:
    Point  center_ = { 0.0, 0.0, 0.0 };
    double radius_ = 0.0;

};  // class Sphere

}  // namespace spica

#endif  // SPICA_SPHERE_H_
