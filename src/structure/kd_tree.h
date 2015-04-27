#ifndef _SPICA_KDTREE_H_
#define _SPICA_KDTREE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifndef SPICA_KDTREE_EXPORT
        #define SPICA_KDTREE_DLL __declspec(dllexport)
    #else
        #define SPICA_KDTREE_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_KDTREE_DLL 
#endif

#include "../geometry/triangle.h"

namespace spica {

    class SPICA_KDTREE_DLL KdTree {
    private:

        struct AxisComparator {
            int dim;
            explicit AxisComparator(int dim_ = 0)
                : dim(dim_)
            {
            }
        };

        struct KdTreeNode {
            int startID;
            int endID;
            KdTreeNode* left;
            KdTreeNode* right;
        };

        Triangle* _triangles;

    public:
        KdTree();
        ~KdTree();
        
        void construct();

    private:
        void constructRec(int startID, int endID, int dim);
    };

}

#endif  // _SPICA_KDTREE_H_
