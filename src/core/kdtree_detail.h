#ifndef _SPICA_KDTREE_DETAIL_H_
#define _SPICA_KDTREE_DETAIL_H_

#include <cmath>
#include <algorithm>

#include "common.h"

namespace spica {

    template <class Ty>
    KdTree<Ty>::KdTree()
        : _nodes(NULL)
        , _numCopies(NULL)
    {
    }

    template <class Ty>
    KdTree<Ty>::KdTree(const KdTree& kdtree)
        : _nodes(NULL)
        , _numCopies(NULL)
    {
        this->operator=(kdtree);
    }

    template <class Ty>
    KdTree<Ty>::~KdTree()
    {
        release();
    }

    template <class Ty>
    KdTree<Ty>& KdTree<Ty>::operator=(const KdTree& kdtree) {
        release();
        _numCopies = kdtree._numCopies;
        (*_numCopies) += 1;
        _nodes = kdtree._nodes;
    }

    template <class Ty>
    void KdTree<Ty>::release() {
        if (_numCopies != NULL) {
            if (*_numCopies == 0) {
                delete[] _nodes;
                delete _numCopies;
                _numCopies = NULL;
            } else {
                (*_numCopies) -= 1;
            }
        }
    }

    template <class Ty>
    void KdTree<Ty>::construct(const std::vector<Ty>& points) {
        // Compute tree size
        const int numPoints = static_cast<int>(points.size());
        int numNodes;
        for (numNodes = 1; numNodes < numPoints; numNodes <<= 1) ;

        _nodes = new KdTreeNode[numNodes];

        std::vector<const Ty*> pointers(numPoints);
        for (int i = 0; i < numPoints; i++) {
            pointers[i] = &points[i];
        }
        constructRec(pointers, 0, 0, numPoints, 0);
        _numCopies = new int(0);
    }

    template <class Ty>
    typename KdTree<Ty>::KdTreeNode* KdTree<Ty>::constructRec(std::vector<const Ty*>& points, const int nodeID, const int startID, const int endID, const int dim) {
        if (startID >= endID) {
            return NULL;
        }

        std::sort(points.begin() + startID, points.begin() + endID, AxisComparator(dim));

        int mid = (startID + endID) / 2;
        KdTreeNode* node = &_nodes[nodeID];
        node->axis = dim;
        node->point = (*points[mid]);
        node->left = constructRec(points, nodeID * 2 + 1, startID, mid, (dim + 1) % 3);
        node->right = constructRec(points, nodeID * 2 + 2, mid + 1, endID, (dim + 1) % 3);
        return node;
    }

    template <class Ty>
    void KdTree<Ty>::knnSearch(const Ty& point, const KnnQuery& query, std::vector<Ty>* results) const {
        PriorityQueue que; 
        KnnQuery qq = query;
        if ((qq.type & EPSILON_BALL) == 0) qq.epsilon = INFTY;

        knnSearchRec(&_nodes[0], point, qq, &que);

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

        const double dist = distance(node->point, point);
        if (dist < query.epsilon) {
            results->push(OrderedType(dist, node->point));
            if ((query.type & K_NEAREST) != 0 && results->size() > query.k) {
                results->pop();

                query.epsilon = distance(results->top().t, point);
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

    template <class Ty>
    double KdTree<Ty>::distance(const Ty& p1, const Ty& p2) {
        return Ty::distance(p1, p2);
    }

    template <>
    inline double KdTree<Vector3D>::distance(const Vector3D& p1, const Vector3D& p2) {
        return (p1 - p2).norm();    
    }

}

#endif  // _SPICA_KDTREE_DETAIL_H_
