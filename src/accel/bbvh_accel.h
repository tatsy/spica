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

#include <memory>

#include "accel_interface.h"

namespace spica {

    /** Binary BVH accelerator class
     *  @ingroup accel_module
     */
    class SPICA_BBVH_ACCEL_DLL BBVHAccel : public IAccel {
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
