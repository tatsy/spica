#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRIANGLE_H_
#define _SPICA_TRIANGLE_H_

#include <array>

#include "shape_interface.h"
#include "../math/vector3d.h"

namespace spica {

    /** Triangle class
     *  @ingroup shape_module
     */
    class SPICA_EXPORTS Triangle : public IShape {
    private:
        std::array<Vector3D, 3> _points;

    public:
        Triangle();
        Triangle(const Vector3D& p0, const Vector3D& p1, const Vector3D& p2);
        Triangle(const Triangle& tri);
        ~Triangle();

        Triangle& operator=(const Triangle& tri);

        Vector3D gravity() const;

        /** Get a vertex with ID in [0, 1, 2]
         */
        Vector3D get(int id) const;

        /** Get a vertex with ID in [0, 1, 2]
         */
        Vector3D operator[](int id) const;

        Vector3D normal() const;

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

        double area() const override;

        Trimesh triangulate() const override;
    };

}  // namespace spica

#endif  // _SPICA_TRIANGLE_H_
