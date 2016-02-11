#ifndef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_QUAD_H_
#define _SPICA_QUAD_H_

#include <array>

#include "shape.h"
#include "../core/point3d.h"

namespace spica {

    /** Quadrangle class
     *  @ingroup shape_module
     */
    class SPICA_EXPORTS Quad : public Shape {
    private:
        std::array<Point, 4> _points;

    public:
        Quad();
        Quad(const Point& v0, const Point& v1, const Point& v2, const Point& v3);
        Quad(const Quad& quad);
        ~Quad();

        Quad& operator=(const Quad& quad);

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

        double area() const override;

        Trimesh triangulate() const override;

        Point operator[](int id) const;
        Point get(int id) const;

        Normal normal() const;

    private:
        Triangle tr(int i, int j, int k) const;
    };

}  // namespace spica

#endif  // _SPICA_QUAD_H_
