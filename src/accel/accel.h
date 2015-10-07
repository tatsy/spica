#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ACCEL_H_
#define _SPICA_ACCEL_H_

#include "kd_tree_accel.h"
#include "bbvh_accel.h"
#include "qbvh_accel.h"

namespace spica {

    //! Accelerator types
    enum class AccelType : int {
        kdtreeAccel,   //! k-d tree acclerator
        bbvhAccel,     //! binary BVH accelerator
        qbvhAccel      //! SIMD-acclerated QBVH acclerator
    };

}  // namespace spica

#endif  // _SPICA_ACCEL_H_
