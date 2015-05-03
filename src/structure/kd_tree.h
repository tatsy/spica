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

#include <vector>

#include "../utils/common.h"
#include "../geometry/bbox.h"
#include "../geometry/triangle.h"


namespace spica {

    struct KdTreeNode {
        int startID;
        int endID;
        BBox bbox;
        KdTreeNode* left;
        KdTreeNode* right;
    };

    class SPICA_KDTREE_DLL KdTree {
    private:

        // ----------------------------------------------
        // Comparator to sort triangles 
        // ----------------------------------------------
        struct AxisComparator {
            int dim;
            explicit AxisComparator(int dim_ = 0)
                : dim(dim_)
            {
                msg_assert(0 <= dim_ && dim_ <= 2, "Dimension must be between 0 and 2");
            }

            bool operator()(const Triangle& t1, const Triangle& t2) const {
                bool ret = false;
                if (dim == 0) {
                    ret = t1.gravity().x() < t2.gravity().x();
                } else if (dim == 1) {
                    ret = t1.gravity().y() < t2.gravity().y();
                } else if (dim == 2) {
                    ret = t1.gravity().z() < t2.gravity().z();
                }
                return ret;
            }
        };

        int _numTriangles;          // # of triangles
        int _numNodes;              // # of nodes
        KdTreeNode* _nodes;         // k-d tree nodes
        Triangle* _triangles;       // triangles contained in the tree

    public:
        KdTree();
        KdTree(const KdTree& kdtree);
        ~KdTree();

        KdTree& operator=(const KdTree& kdtree);
        
        void construct(const std::vector<Triangle>& triangles);

        inline bool empty() const { return _numTriangles == 0; }

        inline const Triangle& getTriangle(int id) const { return _triangles[id]; }

        inline KdTreeNode* root() const { return &_nodes[0]; }

    private:
        void release();
        BBox enclosingBox(int startID, int endID) const;
        KdTreeNode* constructRec(int nodeID, int startID, int endID, int dim);
    };

}

#endif  // _SPICA_KDTREE_H_
