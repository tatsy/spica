#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRIANGLE_H_
#define _SPICA_TRIANGLE_H_

#include <array>

#include "shape.h"
#include "../core/point3d.h"

namespace spica {

    /** Triangle class
     *  @ingroup shape_module
     */
    class SPICA_EXPORTS Triangle : public Shape {
    public:
        Triangle();
        Triangle(const Point& p0, const Point& p1, const Point& p2);
        Triangle(const Triangle& tri);
        ~Triangle();

        Triangle& operator=(const Triangle& tri);

        Point gravity() const;

        /** Get a vertex with ID in [0, 1, 2]
         */
        Point get(int id) const;

        /** Get a vertex with ID in [0, 1, 2]
         */
        Point operator[](int id) const;

        Normal normal() const;

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

        double area() const override;

        Trimesh triangulate() const override;

    private:
        std::array<Point, 3> points_;
    };

}  // namespace spica

#endif  // _SPICA_TRIANGLE_H_
