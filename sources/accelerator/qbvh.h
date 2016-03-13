#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_QBVH_ACCEL_H_
#define _SPICA_QBVH_ACCEL_H_

#include <vector>
#include <xmmintrin.h>

#include "bvh.h"

namespace spica {

/** 
 * Quad BVH accelerator class
 * @ingroup accel_module
 */
class SPICA_EXPORTS QBVHAccel : public BBVHAccel {
public:
    QBVHAccel(const std::vector<std::shared_ptr<Primitive>> &prims);
    virtual ~QBVHAccel();

    bool intersect(Ray& ray, SurfaceInteraction* isect) const override;
    bool intersect(Ray& ray) const override;

private:
    // Private internal classes
    struct SIMDBVHNode;
    union Children;

    // Private methods
    void release();
    void collapse2QBVH(BVHNode* node);

    // Private fields
    std::vector<SIMDBVHNode*> simdNodes_;

};  // class QBVHAccel

}  // namespace spica

#endif  // _SPICA_QBVH_ACCEL_H_

