#ifndef _SPICA_KDTREE_H_
#define _SPICA_KDTREE_H_

#include <vector>

namespace spica {

    template <class Ty>
    class KdTree {
    private:
        struct KdTreeNode {
            Ty* point;
            KdTreeNode* left;
            KdTreeNode* right;
            int axis;
        };

        KdTreeNode* _root;
        int* _numCopies;

    public:
        KdTree();
        KdTree(const KdTree<Ty>& kdtree);
        ~KdTree();

        KdTree& operator=(const KdTree<Ty>& kdtree);

        void construct(const std::vector<Ty>& points);

        void release();

    private:
        void deleteNode(KdTreeNode* node);
        void constructRec(const std::vector<Ty>& points, int startID, int endID, int dim);

    };
}

#include "kdtree_detail.h"

#endif  // _SPICA_KDTREE_H_
