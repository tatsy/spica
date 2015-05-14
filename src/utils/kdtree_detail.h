#ifndef _SPICA_KDTREE_DETAIL_H_
#define _SPICA_KDTREE_DETAIL_H_

#include <cmath>
#include <algorithm>

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
        if (_numCopies != NULL) {
            if (*_numCopies == 0) {
                deleteNode(_root);
                delete _numCopies;
                _numCopies = NULL;
            } else {
                (*_numCopies) -= 1;
            }
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
        std::vector<Ty> temp(points);
        _root = constructRec(temp, 0, numPoints, 0);
        _numCopies = new int(0);
    }

    template <class Ty>
    typename KdTree<Ty>::KdTreeNode* KdTree<Ty>::constructRec(std::vector<Ty>& points, int startID, int endID, int dim) {
        if (startID >= endID) {
            return NULL;
        }

        std::sort(points.begin() + startID, points.begin() + endID, AxisComparator(dim));

        int mid = (startID + endID) / 2;
        KdTreeNode* node = new KdTreeNode();
        node->axis = dim;
        node->point = points[mid];
        node->left = constructRec(points, startID, mid, (dim + 1) % 3);
        node->right = constructRec(points, mid + 1, endID, (dim + 1) % 3);
        return node;
    }

    template <class Ty>
    void KdTree<Ty>::knnSearch(const Ty& point, const KnnQuery& query, std::vector<Ty>* results) const {
        PriorityQueue que; 
        KnnQuery qq = query;
        knnSearchRec(_root, point, qq, &que);

        while (!que.empty()) {
            results->push_back(que.top().t);
            que.pop();
        }
    }

    template <class Ty>
    void KdTree<Ty>::knnSearchRec(typename KdTree<Ty>::KdTreeNode* node, const Ty& point, KnnQuery& query, PriorityQueue* results) const {
        if (node == NULL) {
            return;
        }

        const double dist = (node->point - point).norm();
        if (dist < query.epsilon) {
            results->push(OrderedType(dist, node->point));
            if ((query.type & K_NEAREST) != 0 && results->size() > query.k) {
                results->pop();

                if ((query.type & EPSILON_BALL) == 0) {
                    query.epsilon = dist;
                }
            }
        }

        int axis = node->axis;
        double delta = point.get(axis) - node->point.get(axis);
        if (delta < 0.0) {
            knnSearchRec(node->left, point, query, results);
            if (std::abs(delta) <= query.epsilon) {
                knnSearchRec(node->right, point, query, results);
            }
        } else {
            knnSearchRec(node->right, point, query, results);
            if (std::abs(delta) <= query.epsilon) {
                knnSearchRec(node->left, point, query, results);            
            }
        }
    }

}

#endif  // _SPICA_KDTREE_DETAIL_H_
