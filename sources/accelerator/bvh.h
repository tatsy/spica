#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BBVH_ACCEL_H_
#define _SPICA_BBVH_ACCEL_H_

#include <memory>

#include "accel_interface.h"

namespace spica {

/** Binary BVH accelerator class
    *  @ingroup accel_module
    */
class SPICA_EXPORTS BBVHAccel : public AccelInterface {
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
    std::vector<std::shared_ptr<Primitive>> primitives_;
    BBvhNode* _root;
    //std::vector<Triangle>     _tris;
    std::vector<std::unique_ptr<BBvhNode> > _nodes;
    //void release();

};  // class BBVHAccel

}  // namespace spica

#endif  // _SPICA_BBVH_ACCEL_
