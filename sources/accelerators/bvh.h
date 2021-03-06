#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BBVH_ACCEL_H_
#define _SPICA_BBVH_ACCEL_H_

#include <memory>

#include "core/accelerator.h"
#include "core/renderparams.h"

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
class SPICA_EXPORTS BVHAccel : public Accelerator {
public:
    explicit BVHAccel(const std::vector<std::shared_ptr<Primitive>> &prims,
                      bool useSIMD = false);
    BVHAccel(const std::vector<std::shared_ptr<Primitive>> &prims,
             RenderParams &params);
    virtual ~BVHAccel();

    Bounds3d worldBound() const override;
    void construct() override;
    virtual bool intersect(Ray& ray, SurfaceInteraction* isect) const override;
    virtual bool intersect(Ray& ray) const override;
    std::vector<Triangle> triangulate() const override;

private:
    // Private internal classes
    union Children;
    struct BucketInfo;
    struct SIMDBVHNode;
    struct ComparePoint;
    struct CompareToBucket;

    // Private methods
    bool intersectBVH(Ray &ray, SurfaceInteraction *isect) const;
    bool intersectBVH(Ray &ray) const;
    bool intersectQBVH(Ray &ray, SurfaceInteraction *isect) const;
    bool intersectQBVH(Ray &ray) const;
    
    BVHNode* constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                            int start, int end);
    void release();
    void collapse2QBVH(BVHNode* node);

    // Private fields
    BVHNode* root_;
    std::vector<std::unique_ptr<BVHNode>> nodes_;
    std::vector<SIMDBVHNode*> simdNodes_;
    bool useSIMD_;
};

SPICA_EXPORT_ACCEL_PLUGIN(BVHAccel, "Standard bounding volume hierarchy");

}  // namespace spica

#endif  // _SPICA_BBVH_ACCEL_
