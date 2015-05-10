#ifndef _SPICA_ACCEL_BASE_H_
#define _SPICA_ACCEL_BASE_H_

#include <vector>
#include "../geometry/triangle.h"

namespace spica {

    class AccelBase {
    public:
        virtual ~AccelBase() {}
        virtual void construct(const std::vector<Triangle>& triangles) = 0;
    };

}

#endif  // _SPICA_ACCEL_BASE_H_
