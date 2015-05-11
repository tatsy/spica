#define SPICA_ACCEL_BASE_EXPORT
#include "accel_base.h"

namespace spica {
    
    BBox AccelBase::enclosingBox(const std::vector<Triangle>& triangles) {
        Vector3 posMin(INFTY, INFTY, INFTY);
        Vector3 posMax(-INFTY, -INFTY, -INFTY);
        const int nTri = (int)triangles.size();
        for (int i = 0; i < nTri; i++) {
            for (int j = 0; j < 3; j++) {
                posMin = Vector3::minimum(posMin, triangles[i].p(j));
                posMax = Vector3::maximum(posMax, triangles[i].p(j));
            }
        }
        return BBox(posMin, posMax);
    }

}

