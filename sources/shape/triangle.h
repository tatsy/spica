#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRIANGLE_H_
#define _SPICA_TRIANGLE_H_

#include <array>

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "shape.h"

namespace spica {

class Triangle : public Shape {
public:
    // Public methods
    Triangle();
    Triangle(const Point3D& p0, const Point3D& p1, const Point3D& p2,
             const Transform& objectToWorld = Transform());

    ~Triangle();

    Triangle& operator=(const Triangle& t);

    bool intersect(const Ray& ray, double* tHit,
                   SurfaceInteraction* isect) const override;

    Interaction sample(const Interaction& isect,
                       const Point2D& rands) const override;
    double pdf(const Interaction& pObj, const Vector3D& dir) const override;

    Bound3d worldBound() const override;
    Bound3d objectBound() const override;

    double area() const override;
    std::vector<Triangle> triangulate() const override;

private:
    // Private fields
    std::array<Point3D, 3> points_;
    std::array<Normal,  3> normals_;
    std::array<Point2D, 3> uvs_;

};  // class Triangle

}  // namespace spica

#endif  // _SPICA_TRIANGLE_H_
