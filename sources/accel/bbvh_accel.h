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
private:
    struct BVHPrimitiveInfo;
    struct BucketInfo;
    struct BBvhNode;
    struct ComparePoint;
    struct CompareToBucket;

private:
    BBvhNode* _root;
    std::vector<Triangle>     _tris;
    std::vector<std::unique_ptr<BBvhNode> > _nodes;

public:
    BBVHAccel();
    virtual ~BBVHAccel();

    Bound3d worldBound() const override;
    void construct(const std::vector<Triangle>& triangles) override;
    bool intersect(const Ray& ray, SurfaceInteraction* isect) const override;

private:
    void release();

    BBvhNode* constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                            int start, int end);
};  // class BBVHAccel

}  // namespace spica

#endif  // _SPICA_BBVH_ACCEL_
