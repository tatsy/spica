#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRIANGLE_H_
#define _SPICA_TRIANGLE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_TRIANGLE_EXPORT
        #define SPICA_TRIANGLE_DLL __declspec(dllexport)
    #else
        #define SPICA_TRIANGLE_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_TRIANGLE_DLL
#endif

#include <array>

#include "geometry_interface.h"
#include "../utils/vector3d.h"

namespace spica {

    class SPICA_TRIANGLE_DLL Triangle : public IGeometry {
    private:
        std::array<Vector3D, 3> _points;

    public:
        Triangle();
        Triangle(const Vector3D& p0, const Vector3D& p1, const Vector3D& p2);
        Triangle(const Triangle& tri);
        ~Triangle();

        Triangle& operator=(const Triangle& tri);

        Vector3D gravity() const;

        // get a vertex with ID in [0, 1, 2]
        Vector3D get(int id) const;

        // get a vertex with ID in [0, 1, 2]
        Vector3D operator[](int id) const;

        Vector3D normal() const;

        // Compute ray-triangle intersection with Tomas Moller's algorithm
        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

        double area() const override;

        std::vector<Triangle> triangulate() const override;
    };

}  // namespace spica

#endif  // _SPICA_TRIANGLE_H_
