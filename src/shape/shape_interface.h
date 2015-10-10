/**
 * @defgroup shape_module Shapes
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_SHAPE_INTERFACE_H_
#define SPICA_SHAPE_INTERFACE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #define SPICA_SHAPE_INTERFACE_DLL __declspec(dllexport)
#else
    #define SPICA_SHAPE_INTERFACE_DLL
#endif

#include <vector>

namespace spica {
    
    class Ray;
    class Hitpoint;
    class Triangle;

    /** Shape types
     *  @ingroup shape_module
     *  @brief Enumerator for shape types.
     */
    enum class ShapeType : int {
        None,      /**< None         */
        BBox,      /**< Bounding Box */
        Disk,      /**< Disk         */
        Plane,     /**< Plane        */
        Quad,      /**< Quad         */
        Sphere,    /**< Sphere       */
        Triangle,  /**< Triangle     */
        Trimesh,   /**< Trimesh      */
    };
    
    /** Interface for shapes
     *  @ingroup shape_module
     */
    class SPICA_SHAPE_INTERFACE_DLL IShape {
    protected:
        ShapeType _type;

    public:
        IShape(ShapeType type) : _type{type} {}
        virtual ~IShape() {}
        virtual bool intersect(const Ray& ray, Hitpoint* hitpoint) const = 0;
        virtual double area() const = 0;
        virtual std::vector<Triangle> triangulate() const = 0;
        inline ShapeType type() const { return _type; }
    };

}  // namespace spica

#endif  // SPICA_SHAPE_INTERFACE_H_
