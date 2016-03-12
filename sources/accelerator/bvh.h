#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BBVH_ACCEL_H_
#define _SPICA_BBVH_ACCEL_H_

#include <memory>

#include "accelerator.h"

namespace spica {

/** Binary BVH accelerator class
    *  @ingroup accel_module
    */
class SPICA_EXPORTS BBVHAccel : public Accelerator {
public:
    BBVHAccel(const std::vector<std::shared_ptr<Primitive>> &prims,
              int maxPrimsInNode = 1);
    virtual ~BBVHAccel();

    Bounds3d worldBound() const override;
    void construct() override;
    bool intersect(Ray& ray, SurfaceInteraction* isect) const override;
    bool intersect(Ray& ray) const override;
    std::vector<Triangle> triangulate() const override;

private:
    // Private internal classes
    struct BVHPrimitiveInfo;
    struct BucketInfo;
    struct BBvhNode;
    struct ComparePoint;
    struct CompareToBucket;

    // Private methods
    BBvhNode* constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                            int start, int end);

    // Private fields
    const int maxPrimInNode_;
    BBvhNode* _root;
    std::vector<std::unique_ptr<BBvhNode> > _nodes;

};  // class BBVHAccel

}  // namespace spica

#endif  // _SPICA_BBVH_ACCEL_
