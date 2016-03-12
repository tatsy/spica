#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_QBVH_ACCEL_H_
#define _SPICA_QBVH_ACCEL_H_

#include <vector>
#include <xmmintrin.h>

#include "accel_interface.h"

namespace spica {

/** 
 * Quad BVH accelerator class
 * @ingroup accel_module
 */
class SPICA_EXPORTS QBVHAccel : public AccelInterface {
public:
    QBVHAccel();
    virtual ~QBVHAccel();

    void construct() override;
    bool intersect(Ray& ray, SurfaceInteraction* isect) const override;
    std::vector<Triangle> triangulate() const override;

private:
    // Private internal classes
    struct BVHPrimitiveInfo;
    struct SIMDTrianglePack;
    struct BVHBuildNode;
    struct ComparePoint;
    struct CompareToBucket;
    struct SIMDBVHNode;
    union Children;

    // Private methods
    void release();
    void collapse2QBVH(BVHBuildNode* node);
    BVHBuildNode* constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                                int start, int end, int* totalNodes,
                                std::vector<int>& orderedPrims);

    // Private fields
    BVHBuildNode* root_;
    std::vector<Triangle>  triangles_;
    std::vector<int> ordered_;
    std::vector<SIMDTrianglePack*> simdTris_;
    std::vector<SIMDBVHNode*> simdNodes_;

};  // class QBVHAccel

}  // namespace spica

#endif  // _SPICA_QBVH_ACCEL_H_

