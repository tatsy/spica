#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_accel_interface_H_
#define _SPICA_accel_interface_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_accel_interface_EXPORT
        #define SPICA_accel_interface_DLL __declspec(dllexport)
    #else
        #define SPICA_accel_interface_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_accel_interface_DLL
#endif

#include <vector>
#include <map>

#include "../core/common.h"
#include "../core/uncopyable.h"
#include "../core/forward_decl.h"
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
    class IAccel : private Uncopyable {
    protected:
        struct IndexedTriangle;
        struct AxisComparator;

        AccelType _type;

    public:
        IAccel(AccelType type) : _type(type) {}
        virtual ~IAccel() {}
        virtual void construct(const std::vector<Triangle>& triangles) = 0;
        virtual int  intersect(const Ray& ray, Hitpoint* hitpoint) const = 0;
    };

    struct IAccel::IndexedTriangle {
        int idx;
        Triangle tri;
        IndexedTriangle()
            : idx(-1)
            , tri() {
        }
        IndexedTriangle(int i, const Triangle& t)
            : idx(i)
            , tri(t) {
        }
    };

    struct IAccel::AxisComparator {
        int dim;
        explicit AxisComparator(int dim_ = 0)
            : dim(dim_)
        {
            Assertion(0 <= dim_ && dim_ <= 2, "Dimension must be between 0 and 2");
        }

        bool operator()(const IndexedTriangle& t1, const IndexedTriangle& t2) const {
            return t1.tri.gravity().get(dim) < t2.tri.gravity().get(dim);
        }
    };

}  // namespace spica

#endif  // _SPICA_accel_interface_H_
