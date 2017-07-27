#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BBVH_ACCEL_H_
#define _SPICA_BBVH_ACCEL_H_

#include <memory>

#include "core/accelerator.h"

namespace spica {

struct BVHPrimitiveInfo {
    int primIdx;
    Point3d centroid;
    Bounds3d bounds;
    BVHPrimitiveInfo() {}
    BVHPrimitiveInfo(int pid, const Bounds3d& b)
        : primIdx(pid)
        , centroid()
        , bounds(b) {
        centroid = (b.posMax() + b.posMin()) * 0.5;
    }
};

struct BVHNode {
    Bounds3d bounds;
    BVHNode* left;
    BVHNode* right;
    int splitAxis;
    int primIdx;

    void initLeaf(const Bounds3d& b, int pid) {
        this->bounds = b;
        this->splitAxis = 0;
        this->primIdx = pid;
    }

    void initFork(const Bounds3d& b, BVHNode* l, BVHNode* r, int axis) {
        this->bounds = b;
        this->left  = l;
        this->right = r;
        this->splitAxis = axis;
        this->primIdx = -1;
    }

    bool isLeaf() const {
        return primIdx >= 0;
    }
};

/** Binary BVH accelerator class
 *  @ingroup accel_module
 */
class SPICA_EXPORTS BBVHAccel : public Accelerator {
public:
    BBVHAccel(const std::vector<std::shared_ptr<Primitive>> &prims);
    virtual ~BBVHAccel();

    Bounds3d worldBound() const override;
    void construct() override;
    virtual bool intersect(Ray& ray, SurfaceInteraction* isect) const override;
    virtual bool intersect(Ray& ray) const override;
    std::vector<Triangle> triangulate() const override;

protected:
    // Private internal classes
    struct BucketInfo;
    struct ComparePoint;
    struct CompareToBucket;

    // Private methods
    BVHNode* constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                            int start, int end);

    // Private fields
    BVHNode* root_;
    std::vector<std::unique_ptr<BVHNode> > nodes_;

};  // class BBVHAccel

}  // namespace spica

#endif  // _SPICA_BBVH_ACCEL_
