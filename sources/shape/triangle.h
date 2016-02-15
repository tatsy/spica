#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRIANGLE_H_
#define _SPICA_TRIANGLE_H_

#include <array>

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/point2d.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"

#include "shape.h"

namespace spica {

class SPICA_EXPORTS Triangle : public Shape {
public:
    // Public methods
    Triangle();
    Triangle(const Point3D& p0, const Point3D& p1, const Point3D& p2,
             const Transform& objectToWorld = Transform());
    Triangle(const Point3D& p0, const Point3D& p1, const Point3D& p2,
             const Normal3D& n0, const Normal3D& n1, const Normal3D& n2,
             const Transform& objectToWorld = Transform());
    Triangle(const Point3D& p0, const Point3D& p1, const Point3D& p2,
             const Normal3D& n0, const Normal3D& n1, const Normal3D& n2,
             const Point2D& uv0, const Point2D& uv1, const Point2D& uv2,
             const Transform& objectToWorld = Transform());
    Triangle(const Triangle& t);

    ~Triangle();

    Triangle& operator=(const Triangle& t);
    const Point3D& operator[](int i) const;

    bool intersect(const Ray& ray, double* tHit,
                   SurfaceInteraction* isect) const override;

    Interaction sample(const Interaction& isect,
                       const Point2D& rands) const override;

    Bound3d objectBound() const override;

    double area() const override;
    std::vector<Triangle> triangulate() const override;

    Point3D gravity() const;
    const Normal3D& normal(int i) const;
    const Point2D&  uv(int i) const;

private:
    // Private fields
    std::array<Point3D, 3> points_;
    std::array<Normal,  3> normals_;
    std::array<Point2D, 3> uvs_;

};  // class Triangle

}  // namespace spica

#endif  // _SPICA_TRIANGLE_H_
