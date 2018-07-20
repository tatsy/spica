#ifndef _SPICA_KDTREE_DETAIL_H_
#define _SPICA_KDTREE_DETAIL_H_

#include <cmath>
#include <stack>
#include <algorithm>

#include "core/common.h"
#include "core/vector3d.h"

namespace spica {

    template <class Ty>
    KdTree<Ty>::KdTree()
        : nodes_(nullptr)
        , numCopies_(nullptr)
    {
    }

    template <class Ty>
    KdTree<Ty>::KdTree(const KdTree& kdtree)
        : nodes_(nullptr)
        , numCopies_(nullptr)
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
        numCopies_ = kdtree.numCopies_;
        (*numCopies_) += 1;
        nodes_ = kdtree.nodes_;
    }

    template <class Ty>
    void KdTree<Ty>::release() {
        if (numCopies_ != nullptr) {
            if (*numCopies_ == 0) {
                delete[] nodes_;
                delete numCopies_;
                numCopies_ = nullptr;
            } else {
                (*numCopies_) -= 1;
            }
        }
    }

    template <class Ty>
    void KdTree<Ty>::add(const Ty& point) {
        nodes_ = addRec(nodes_, point, 0);
    }

    template <class Ty>
    typename KdTree<Ty>::KdTreeNode* KdTree<Ty>::addRec(KdTreeNode* node, const Ty& point, int dim) {
        if (node == nullptr) {
            KdTreeNode* ret = new KdTreeNode;
            ret->point = point;
            ret->axis = dim;
            return ret;
        } else {
            if (point[dim] < node->point[dim]) {
                node->left = addRec(node->left, point, (dim + 1) % 3);
            } else {
                node->right = addRec(node->right, point, (dim + 1) % 3);
            }
            return node;
        }
    }

    template <class Ty>
    void KdTree<Ty>::construct(const std::vector<Ty>& points) {
        // Compute tree size
        const int numPoints = static_cast<int>(points.size());
        int numNodes;
        for (numNodes = 1; numNodes < numPoints; numNodes <<= 1) ;

        nodes_ = new KdTreeNode[numNodes];

        std::vector<const Ty*> pointers(numPoints);
        for (int i = 0; i < numPoints; i++) {
            pointers[i] = &points[i];
        }
        constructRec(pointers, 0, 0, numPoints, 0);
        numCopies_ = new int(0);
    }

    template <class Ty>
    typename KdTree<Ty>::KdTreeNode* KdTree<Ty>::constructRec(std::vector<const Ty*>& points, const int nodeID, const int startID, const int endID, const int dim) {
        if (startID >= endID) {
            return NULL;
        }

        const int mid = (startID + endID) / 2;
        std::nth_element(points.begin() + startID, points.begin() + mid, points.begin() + endID, AxisComparator(dim));

        KdTreeNode* node = &nodes_[nodeID];
        node->axis = dim;
        node->point = (*points[mid]);
        node->left = constructRec(points, nodeID * 2 + 1, startID, mid, (dim + 1) % 3);
        node->right = constructRec(points, nodeID * 2 + 2, mid + 1, endID, (dim + 1) % 3);
        return node;
    }

    template <class Ty>
    void KdTree<Ty>::knnSearch(const Ty& point, const KnnQuery& q, std::vector<Ty>* results) const {
        // Queue to store result
        PriorityQueue que;
        KnnQuery query = q;
        if ((query.type & EPSILON_BALL) == 0) {
            query.epsilon = INFTY;
        } else {
            query.epsilon = query.epsilon * query.epsilon;
        }

        // Stack-based depth first search
        using NodeType = KdTree<Ty>::KdTreeNode;
        NodeType *nodes[64];
        int pos = 0;
        nodes[pos++] = &nodes_[0];

        while (pos > 0) {
            NodeType *node = nodes[--pos];

            if (node == nullptr) {
                continue;
            }

            const double dist2 = squaredDistance(node->point, point);

            if (dist2 < query.epsilon * query.epsilon) {
                que.push(OrderedType(dist2, node->point));
                if ((query.type & K_NEAREST) != 0 && (int)que.size() > query.k) {
                    que.pop();
                    query.epsilon = std::min(query.epsilon, squaredDistance(que.top().t, point));
                }
            }

            const int axis = node->axis;
            const double delta = point[axis] - node->point[axis];
            if (delta < 0.0) {
                if (std::abs(delta) < query.epsilon) {
                    nodes[pos++] = node->right;
                }
                nodes[pos++] = node->left;
            } else {
                if (std::abs(delta) < query.epsilon) {
                    nodes[pos++] = node->left;
                }
                nodes[pos++] = node->right;
            }
        }

        // Store results
        while (!que.empty()) {
            results->push_back(que.top().t);
            que.pop();
        }
    }

    template <class Ty>
    double KdTree<Ty>::squaredDistance(const Ty& p1, const Ty& p2) {
        return Ty::squaredDistance(p1, p2);
    }

    template <>
    inline double KdTree<Vector3d>::squaredDistance(const Vector3d& p1, const Vector3d& p2) {
        return (p1 - p2).squaredNorm();
    }

}

#endif  // _SPICA_KDTREE_DETAIL_H_
