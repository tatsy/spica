#define SPICA_ACCEL_BASE_EXPORT
#include "accel_base.h"

namespace spica {
    
    BBox AccelBase::enclosingBox(const std::vector<IndexedTriangle>& triangles) {
        Vector3D posMin(INFTY, INFTY, INFTY);
        Vector3D posMax(-INFTY, -INFTY, -INFTY);
        const int nTri = (int)triangles.size();
        for (int i = 0; i < nTri; i++) {
            for (int j = 0; j < 3; j++) {
                posMin = Vector3D::minimum(posMin, triangles[i].tri.get(j));
                posMax = Vector3D::maximum(posMax, triangles[i].tri.get(j));
            }
        }
        return BBox(posMin, posMax);
    }

}  // namespace spica

