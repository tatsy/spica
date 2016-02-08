#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_QBVH_ACCEL_H_
#define _SPICA_QBVH_ACCEL_H_

#include <vector>
#include <xmmintrin.h>

#include "accel_interface.h"

namespace spica {

    /** Quad BVH accelerator class
     *  @ingroup accel_module
     */
    class SPICA_EXPORTS QBVHAccel : public IAccel {
    private:

        struct BVHPrimitiveInfo;
        struct SIMDTrianglePack;

        struct BVHBuildNode;
        struct ComparePoint;
        struct CompareToBucket;
        struct SIMDBVHNode;
        union Children;

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

