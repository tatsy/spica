/**
 * @defgroup shape_module Shapes
 * @brief Shape classes.
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_SHAPE_H_
#define SPICA_SHAPE_H_

#include <vector>

#include "../core/common.h"
#include "../core/forward_decl.h"

namespace spica {
    
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
    
    /** Abstract shape class.
     *  @ingroup shape_module
     */
    class SPICA_EXPORTS Shape {
    public:
        Shape(ShapeType type) : type_{ type } {}
        virtual ~Shape() {}
        virtual bool intersect(const Ray& ray, Hitpoint* hitpoint) const = 0;

        virtual Interaction sample(const Interaction& isect, const Point2D& rands);
        virtual double pdf(const Interaction& pObj, const Vector3D& dir);

        virtual double area() const = 0;
        virtual Trimesh triangulate() const = 0;
        inline ShapeType type() const { return type_; }

    protected:
        ShapeType type_;
    };

}  // namespace spica

#endif  // SPICA_SHAPE_INTERFACE_H_
