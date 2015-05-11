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

#include "accel_base.h"
#include "../utils/common.h"
#include "../utils/uncopyable.h"
#include "../geometry/bbox.h"

namespace spica {

    class SPICA_KDTREE_ACCEL_DLL KdTreeAccel : public AccelBase {
    private:
        struct KdTreeNode : public Uncopyable {
            BBox bbox;
            unsigned int numTriangles;
            Triangle* triangles;
            KdTreeNode* left;
            KdTreeNode* right;
            bool isLeaf;

            KdTreeNode()
                : bbox()
                , numTriangles(0)
                , triangles(NULL)
                , left(NULL)
                , right(NULL)
                , isLeaf(false)
            {
            }

            ~KdTreeNode()
            {
                delete[] triangles;
            }
        };

        static const int _maxNodeSize = 2;
        KdTreeNode* _root;          // tree root
        unsigned int* _numCopies;   // # of tree copies

    public:
        KdTreeAccel();
        KdTreeAccel(const KdTreeAccel& kdtree);
        ~KdTreeAccel();

        KdTreeAccel& operator=(const KdTreeAccel& kdtree);
        
        void construct(const std::vector<Triangle>& triangles);
        bool intersect(const Ray& ray, Hitpoint* hitpoint) const;

    private:
        void release();
        void deleteNode(KdTreeNode* node);
        KdTreeNode* constructRec(std::vector<Triangle>& triangles, int dim);

        static bool intersectRec(KdTreeNode* node, const Ray& ray, Hitpoint* hitpoint, double tMin, double tMax);
    };

}

#endif  // _SPICA_KDTREE_H_
