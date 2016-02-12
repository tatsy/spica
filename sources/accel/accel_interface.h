#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ACCEL_INTERFACE_H_
#define _SPICA_ACCEL_INTERFACE_H_

#include <vector>
#include <map>

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "../core/uncopyable.h"
#include "../core/primitive.h"
#include "../core/bound3d.h"
#include "../shape/triangle.h"

namespace spica {

/** Accelerator type enum
 *  @ingroup accel_module
 *  @brief Enumerator for acclerator types.
 */
enum class AccelType : int {
    KdTree,   /**< K-D tree acclerator */
    BBVH,     /**< binary BVH accelerator */
    QBVH      /**< SIMD-acclerated QBVH acclerator */
};

/** Interface for intersection test accelerators.
    *  @ingroup accel_module
    */
class AccelInterface : public Aggregate, Uncopyable {
public:
    // Public methods
    AccelInterface(AccelType type) : type_{ type } {}
    virtual ~AccelInterface() {}
    virtual void construct(const std::vector<Triangle>& triangles) = 0;

protected:
    // Protected internal classes
    struct IndexedTriangle;
    struct AxisComparator;

    // Protected fields
    AccelType type_;
    Bound3d   worldBound_;
};

struct AccelInterface::IndexedTriangle {
    IndexedTriangle()
        : idx(-1)
        , tri() {
    }
    IndexedTriangle(int i, const Triangle& t)
        : idx(i)
        , tri(t) {
    }

    int idx;
    Triangle tri;
};

struct AccelInterface::AxisComparator {
    int dim;
    explicit AxisComparator(int dim_ = 0)
        : dim(dim_)
    {
        Assertion(0 <= dim_ && dim_ <= 2, "Dimension must be between 0 and 2");
    }

    bool operator()(const IndexedTriangle& t1, const IndexedTriangle& t2) const {
        return t1.tri.gravity()[dim] < t2.tri.gravity()[dim];
    }
};

}  // namespace spica

#endif  // _SPICA_ACCEL_INTERFACE_H_
