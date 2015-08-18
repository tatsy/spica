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
            std::vector<TriangleWithID> triangles;
            KdTreeNode* left;
            KdTreeNode* right;
            bool isLeaf;

            KdTreeNode()
                : bbox()
                , triangles()
                , left(NULL)
                , right(NULL)
                , isLeaf(false)
            {
            }

            ~KdTreeNode()
            {
            }
        };

        static const int _maxNodeSize = 2;
        KdTreeNode* _root;          // tree root

    public:
        KdTreeAccel();
        KdTreeAccel(const KdTreeAccel& kdtree);     // Copy
        KdTreeAccel(KdTreeAccel&& kdtree);          // Move
        ~KdTreeAccel();

        KdTreeAccel& operator=(const KdTreeAccel& kdtree);  // Copy
        KdTreeAccel& operator=(KdTreeAccel&& kdtree);       // Move
        
        void construct(const std::vector<Triangle>& triangles);
        int  intersect(const Ray& ray, Hitpoint* hitpoint) const override;

    private:
        void release();
        void deleteNode(KdTreeNode* node);
        KdTreeNode* copyNode(KdTreeNode* node);
        KdTreeNode* constructRec(std::vector<TriangleWithID>& triangles, int dim);

        static int intersectRec(KdTreeNode* node, const Ray& ray, Hitpoint* hitpoint, double tMin, double tMax);
    };

}

#endif  // _SPICA_KDTREE_H_
