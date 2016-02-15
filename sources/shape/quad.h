#ifndef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_QUAD_H_
#define _SPICA_QUAD_H_

#include <array>

#include "shape.h"
#include "../core/point3d.h"

namespace spica {

/**
 * Quadrangle class
 * @ingroup shape_module
 */
class SPICA_EXPORTS Quad : public Shape {
public:
    // Public methods
    Quad();
    Quad(const Point& v0, const Point& v1, const Point& v2, const Point& v3,
         const Transform& objectToWorld = Transform());
    Quad(const Quad& quad);
    ~Quad();

    Quad& operator=(const Quad& quad);
    Point operator[](int id) const;

    bool intersect(const Ray& ray, double* tHit,
                    SurfaceInteraction* isect) const override;

    Interaction sample(const Interaction& isect,
                       const Point2D& rands) const override;

    Bound3d objectBound() const override;

    double area() const override;
    std::vector<Triangle> triangulate() const override;


private:
    // Private methods
    Triangle tr(int i, int j, int k) const;

    // Private fields
    std::array<Point, 4> points_;

};  // class Quad

}  // namespace spica

#endif  // _SPICA_QUAD_H_
