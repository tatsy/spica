#ifndef _SPICA_KDTREE_DETAIL_H_
#define _SPICA_KDTREE_DETAIL_H_

namespace spica {

    template <class Ty>
    KdTree<Ty>::KdTree()
        : _root(NULL)
        , _numCopies(NULL)
    {
    }

    template <class Ty>
    KdTree<Ty>::KdTree(const KdTree& kdtree)
        : _root(NULL)
        , _numCopies(NULL)
    {
    }

    template <class Ty>
    KdTree<Ty>::~KdTree()
    {
        release();
    }

    template <class Ty>
    void KdTree<Ty>::release() {
        if (*_numCopies == 0) {
            deleteNode(_root);
            delete _numCopies;
            _numCopies = NULL;
        } else {
            (*_numCopies) -= 1;
        }
    }

    template <class Ty>
    void KdTree<Ty>::deleteNode(KdTreeNode* node) {
        if (node->left != NULL) {
            deleteNode(node->left);
        }
        if (node->right != NULL) {
            deleteNode(node->right);
        }
        delete node;
    }

    template <class Ty>
    void KdTree<Ty>::construct(const std::vector<Ty>& points) {
        const int numPoints = static_cast<int>(points.size());
        constructRec(points, 0, numPoints, 0);
    }

    template <class Ty>
    void KdTree<Ty>::constructRec(const std::vector<Ty>& points, int startID, int endID, int dim) {
        std::sort(points.begin() + startID, points.begin() + endID, Ty::compare(dim));
    }
}

#endif  // _SPICA_KDTREE_DETAIL_H_
