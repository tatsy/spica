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

#include "accel_base.h"

namespace spica {

    /** Quad BVH accelerator class
     *  @ingroup accel_module
     */
    class SPICA_QBVH_ACCEL_DLL QBVHAccel : public AccelBase {
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

