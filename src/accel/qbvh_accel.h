#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_QBVH_ACCEL_H_
#define _SPICA_QBVH_ACCEL_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_QBVH_ACCEL_EXPORT
        #define SPICA_QBVH_ACCEL_DLL __declspec(dllexport)
    #else
        #define SPICA_QBVH_ACCEL_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_QBVH_ACCEL_DLL
#endif

#include <vector>
#include <xmmintrin.h>

#include "../core/uncopyable.h"
#include "accel_base.h"

namespace spica {

    /** Quad BVH accelerator class
     * @ingroup accel_module
     */
    class SPICA_QBVH_ACCEL_DLL QBVHAccel : public AccelBase {
    private:

        struct BVHPrimitiveInfo {
            int primitiveNumber;
            Vector3D centroid;
            BBox bounds;

            BVHPrimitiveInfo(int pn, const BBox& b) 
                : primitiveNumber(pn)
                , bounds(b) {
                centroid = (b.posMin() + b.posMax()) * 0.5;
            }
        };

        struct SIMDTrianglePack {
            __m128 x[3];
            __m128 y[3];
            __m128 z[3];
            int idx[4];
        };

        struct BVHBuildNode {
            BBox bounds;
            BVHBuildNode* children[2];
            int splitAxis, firstPrimOffset, nPrimitives;
            int simdTrisIdx;

            BVHBuildNode()
                : bounds()
                , children()
                , splitAxis(-1)
                , firstPrimOffset(-1)
                , nPrimitives(0) 
                , simdTrisIdx(-1) {
                children[0] = children[1] = nullptr;
            }

            void InitLeaf(int first, int n, const BBox& b, const int asimdTrisIdx) {
                firstPrimOffset = first;
                nPrimitives = n;
                bounds = b;
                simdTrisIdx = asimdTrisIdx;
                splitAxis = 0;
            }

            void InitInterior(int axis, BVHBuildNode* c0, BVHBuildNode* c1) {
                children[0] = c0;
                children[1] = c1;
                bounds = BBox::merge(c0->bounds, c1->bounds);
                splitAxis = axis;
                firstPrimOffset = -1;
                nPrimitives = 0;
            }
        };

        struct ComparePoint {
            int dim;
            ComparePoint(int d) : dim(d) {}
            bool operator()(const BVHPrimitiveInfo &a, const BVHPrimitiveInfo &b) const {
                return a.centroid.get(dim) < b.centroid.get(dim);
            }
        };

        struct CompareToBucket {
            int splitBucket, nBuckets, dim;
            const BBox& centroidBounds;

            CompareToBucket(int split, int num, int d, const BBox& b)
                : centroidBounds(b)
                , splitBucket(split)
                , nBuckets(num)
                , dim(d) {
            }

            bool operator()(const BVHPrimitiveInfo& p) const {
                int b = (int)(nBuckets * ((p.centroid.get(dim) - centroidBounds.posMin().get(dim)) / (centroidBounds.posMax().get(dim) - centroidBounds.posMin().get(dim))));
                if (b == nBuckets) {
                    b = nBuckets - 1;
                }
                return b <= splitBucket;
            }
        };

        union Children { 
            struct Node {
                unsigned flag  : 1;
                unsigned index : 31;
            } node;

            struct Leaf {
                unsigned flag : 1;
                unsigned nPrimitives: 3;
                unsigned index : 28;
            } leaf;

            unsigned int raw;
        };

        struct SIMDBVHNode {
            __m128 bboxes[2][3];
            Children children[4];
            int axis_top;
            int axis_left;
            int axis_right;
            int reserved;
        };

        BVHBuildNode* _root;

        std::vector<Triangle>  _triangles;
        std::vector<int> _ordered;
        std::vector<SIMDTrianglePack*> _simdTris;
        std::vector<SIMDBVHNode*> _simdNodes;

    public:
        QBVHAccel();
        ~QBVHAccel();

        void construct(const std::vector<Triangle>& triangles) override;

        int intersect(const Ray& ray, Hitpoint* hitpoint) const override;

    private:
        void release();
        
        BVHBuildNode* constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                                   int start, int end, int* totalNodes,
                                   std::vector<int>& orderedPrims);

        void collapse2QBVH(BVHBuildNode* node);
   };

}  // namespace spica

#endif  // _SPICA_QBVH_ACCEL_H_

