#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BBVH_ACCEL_
#define _SPICA_BBVH_ACCEL_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_BBVH_ACCEL_EXPORT
        #define SPICA_BBVH_ACCEL_DLL __declspec(dllexport)
    #else
        #define SPICA_BBVH_ACCEL_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_BBVH_ACCEL_DLL
#endif

#include "accel_base.h"

namespace spica {

    /** Binary BVH accelerator class
     * @ingroup accel_module
     */
    class SPICA_BBVH_ACCEL_DLL BBVHAccel : public AccelBase {
    private:
        struct BVHPrimitiveInfo {
            int primIdx;
            Vector3D centroid;
            BBox bounds;

            BVHPrimitiveInfo(int pid, const BBox& b) 
                : primIdx(pid)
                , centroid()
                , bounds(b) {
                centroid = (b.posMax() + b.posMin()) * 0.5;
            }
        };

        struct BucketInfo {
            int count;
            BBox bounds;
            BucketInfo() 
                : count(0)
                , bounds() {
            }
        };

        struct BBvhNode {
            BBox bounds;
            BBvhNode* left;
            BBvhNode* right;
            int splitAxis;
            int triIdx;

            void initLeaf(const BBox& b, int tid) {
                this->bounds    = bounds;
                this->left      = nullptr;
                this->right     = nullptr;
                this->splitAxis = -1;
                this->triIdx    = tid;
            }

            void initFork(const BBox& b, BBvhNode* l, BBvhNode* r, int axis) {
                this->bounds    = b;
                this->left      = l;
                this->right     = r;
                this->splitAxis = axis;
                this->triIdx    = -1;
            }

            bool isLeaf() const {
                return triIdx >= 0;
            }
        };

        struct ComparePoint {
            int dim;
            ComparePoint(int d) : dim(d) {}
            bool operator()(const BVHPrimitiveInfo& a,
                            const BVHPrimitiveInfo& b) const {
                return a.centroid.get(dim) < b.centroid.get(dim);
            }
        };

        struct CompareToBucket {
            int splitBucket, nBuckets, dim;
            const BBox& centroidBounds;

            CompareToBucket(int split, int num, int d, const BBox& b)
                : splitBucket(split)
                , nBuckets(num)
                , dim(d)
                , centroidBounds(b) {
            }

            bool operator()(const BVHPrimitiveInfo& p) const {
                const double cmin = centroidBounds.posMin().get(dim);
                const double cmax = centroidBounds.posMax().get(dim);
                const double inv  = (1.0) / (std::abs(cmax - cmin) + EPS);
                const double diff = std::abs(p.centroid.get(dim) - cmin);
                int b = static_cast<int>(nBuckets * diff * inv);
                if (b >= nBuckets) {
                    b = nBuckets - 1;
                }
                return b <= splitBucket;
            }
        };

    private:
        BBvhNode* _root;
        std::vector<Triangle> _tris;
        std::vector<BBvhNode*> _nodes;

    public:
        BBVHAccel();
        ~BBVHAccel();

        void construct(const std::vector<Triangle>& triangles) override;

        int intersect(const Ray& ray, Hitpoint* hpoint) const override;

    private:
        void release();

        BBvhNode* constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                               int start, int end);
    };

}  // namespace spica

#endif  // _SPICA_BBVH_ACCEL_
