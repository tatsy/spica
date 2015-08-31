#ifndef _SPICA_ACCEL_H_
#define _SPICA_ACCEL_H_

#include "kd_tree_accel.h"
#include "qbvh_accel.h"

namespace spica {

    enum class AccelType : int {
        kdtreeAccel,
        qbvhAccel
    };

}

#endif  // _SPICA_ACCEL_H_
