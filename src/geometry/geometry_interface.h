#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_GEOMETRY_INTERFACE_H_
#define SPICA_GEOMETRY_INTERFACE_H_

#if defined(_WIN32) || defined(__WIN32__)
#define SPICA_GEOMETRY_INTERFACE_DLL __declspec(dllexport)
#else
#define SPICA_GEOMETRY_INTERFACE_DLL
#endif

#include <vector>

namespace spica {
    
    // Forward declarations
    class Ray;
    class Hitpoint;
    class Triangle;
    
    // --------------------------------------------------
    // Interface class for geometries
    // --------------------------------------------------
    class SPICA_GEOMETRY_INTERFACE_DLL IGeometry {    
    public:
        virtual ~IGeometry() {}
        virtual bool intersect(const Ray& ray, Hitpoint* hitpoint) const = 0;
        virtual double area() const = 0;
        virtual std::vector<Triangle> triangulate() const = 0;
    };

}  // namespace spica

#endif  // SPICA_PRIMITIVE_H_
