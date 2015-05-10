#ifndef _SPICA_QBVH_ACCEL_H_
#define _SPICA_QBVH_ACCEL_H_

#include <xmmintrin.h>

#include "accel_base.h"

namespace spica {

    class QBVHAccel : public AccelBase {
    private:
        struct QBVHNode {
            __m128 childBoxes[2][3];  // [min-max][x-y-z]
            QBVHNode* children[4];    // Child nodes
            int axisTop;
            int axisLeft;
            int axisRight;
        };

        QBVHAccel();

        void construct(const std::vector<Triangle>& triangles);

    private:
        QBVHNode* constructRec(const std::vector<Triangle>& triangles, int dim);

    };

}

#endif  // _SPICA_QBVH_ACCEL_H_

