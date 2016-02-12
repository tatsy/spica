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

#include "../math/transform.h"

namespace spica {
    
/** Shape types
 *  @ingroup shape_module
 *  @brief Enumerator for shape types.
 */
enum class ShapeType : int {
    None,      /**< None         */
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
    // Public methods
    Shape(const Transform& objectToWorld, ShapeType type)
        : objectToWorld_{ objectToWorld }
        , worldToObject_{ objectToWorld.inverted() }
        , type_{ type } {
    }

    virtual ~Shape() {}
    virtual bool intersect(const Ray& ray, double* tHit,
                           SurfaceInteraction* isect) const = 0;

    virtual Interaction sample(const Interaction& isect,
                               const Point2D& rands) const;
    virtual double pdf(const Interaction& pObj, const Vector3D& dir) const;

    virtual Bound3d worldBound() const = 0;
    virtual Bound3d objectBound() const = 0;

    virtual double area() const = 0;
    virtual std::vector<Triangle> triangulate() const = 0;

    inline ShapeType type() const { return type_; }

protected:
    // Protected fields
    const Transform objectToWorld_, worldToObject_;
    ShapeType type_;

};  // class Shape

}  // namespace spica

#endif  // SPICA_SHAPE_INTERFACE_H_
