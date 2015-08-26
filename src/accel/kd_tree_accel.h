#ifndef _SPICA_KDTREE_ACCEL_H_
#define _SPICA_KDTREE_ACCEL_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_KDTREE_ACCEL_EXPORT
        #define SPICA_KDTREE_ACCEL_DLL __declspec(dllexport)
    #else
        #define SPICA_KDTREE_ACCEL_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_KDTREE_ACCEL_DLL 
#endif

#include <vector>

#include "accel_base.h"
#include "../utils/common.h"
#include "../utils/uncopyable.h"
#include "../geometry/bbox.h"

namespace spica {

    class SPICA_KDTREE_ACCEL_DLL KdTreeAccel : public AccelBase {
    private:
        struct KdTreeNode : public Uncopyable {
            BBox bbox;
            IndexedTriangle triangle;
            KdTreeNode* left;
            KdTreeNode* right;
            bool isLeaf;

            KdTreeNode()
                : bbox()
                , triangle()
                , left(NULL)
                , right(NULL)
                , isLeaf(false)
            {
            }

            ~KdTreeNode()
            {
            }
        };

        KdTreeNode* _root;          // tree root

    public:
        KdTreeAccel();
        ~KdTreeAccel();
        
        void construct(const std::vector<Triangle>& triangles);
        int  intersect(const Ray& ray, Hitpoint* hitpoint) const override;

    private:
        void release();
        void deleteNode(KdTreeNode* node);
        KdTreeNode* copyNode(KdTreeNode* node);
        KdTreeNode* constructRec(std::vector<IndexedTriangle>& triangles, int start, int end);
    };

}

#endif  // _SPICA_KDTREE_H_
