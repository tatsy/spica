/**
 * @defgroup accel_module Accel
 * @brief The class module for intersecting accelerating data structures
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ACCEL_H_
#define _SPICA_ACCEL_H_

#include "kd_tree_accel.h"
#include "bbvh_accel.h"
#include "qbvh_accel.h"

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

}  // namespace spica

#endif  // _SPICA_ACCEL_H_
