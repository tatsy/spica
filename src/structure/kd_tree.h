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
#include "../utils/uncopyable.h"
#include "../geometry/bbox.h"
#include "../geometry/triangle.h"


namespace spica {

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

        static const int _maxNodeSize = 5;
        KdTreeNode* _root;          // tree root
        unsigned int* _numCopies;   // # of tree copies

    public:
        KdTree();
        KdTree(const KdTree& kdtree);
        ~KdTree();

        KdTree& operator=(const KdTree& kdtree);
        
        void construct(std::vector<Triangle>& triangles);

        inline bool empty() const { return _root == 0; }

        inline KdTreeNode* root() const { return _root; }

    private:
        void release();
        void deleteNode(KdTreeNode* node);
        KdTreeNode* constructRec(std::vector<Triangle>& triangles, int dim);

        static BBox enclosingBox(const std::vector<Triangle>& triangles);
    };

}

#endif  // _SPICA_KDTREE_H_
