#define SPICA_KDTREE_ACCEL_EXPORT
#include "kd_tree_accel.h"

#include <cstring>
#include <stack>
#include <algorithm>

namespace spica {

    KdTreeAccel::KdTreeAccel() 
        : IAccel{AccelType::KdTree}
        , _root{nullptr} {
    }

    KdTreeAccel::~KdTreeAccel() {
        release();
    }

    void KdTreeAccel::release() {
        deleteNode(_root);
    }

    void KdTreeAccel::deleteNode(KdTreeNode* node) {
        if (node != nullptr) {
            if (node->left != nullptr) {
                deleteNode(node->left);
            }

            if (node->right != nullptr) {
                deleteNode(node->right);
            }

            delete node;
        }
    }

    KdTreeAccel::KdTreeNode* KdTreeAccel::copyNode(KdTreeNode* node) {
        KdTreeNode* ret = nullptr;
        if (node != nullptr) {
            ret = new KdTreeNode();
            ret->bbox = node->bbox;
            node->triangle = ret->triangle;
            ret->isLeaf = node->isLeaf;

            node->left = copyNode(node->left);
            node->right = copyNode(node->right);
        }
        return ret;
    }

    void KdTreeAccel::construct(const std::vector<Triangle>& triangles) {
        release();

        const int numTriangles = static_cast<int>(triangles.size());
        std::vector<IndexedTriangle> temp(numTriangles);
        for (int i = 0; i < numTriangles; i++) {
            temp[i].tri = triangles[i];
            temp[i].idx = i;
        }
        _root = constructRec(temp, 0, temp.size());
    }

    KdTreeAccel::KdTreeNode* KdTreeAccel::constructRec(std::vector<IndexedTriangle>& triangles, int start, int end) {
        if (start == end) {
            return nullptr;
        }

        if (start + 1 == end) {
            KdTreeNode* node = new KdTreeNode();
            node->isLeaf = true;
            node->left = nullptr;
            node->right = nullptr;
            node->triangle = triangles[start];
            node->bbox = BBox::fromTriangle(triangles[start].tri);
            return node;
        }

        BBox bounds;
        for (int i = start; i < end; i++) {
            bounds.merge(triangles[i].tri);
        }
        const int dim = bounds.maximumExtent();

        // Sort triangles
        std::sort(triangles.begin() + start, triangles.begin() + end, AxisComparator(dim));

        const int mid = (start + end) / 2;

        KdTreeNode* node = new KdTreeNode();
        node->bbox = bounds;
        node->left = constructRec(triangles, start, mid);
        node->right = constructRec(triangles, mid, end);
        node->isLeaf = false;

        return node;
    }

    int KdTreeAccel::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        int tid = -1;

        std::stack<KdTreeNode*> todoNode;
        todoNode.push(_root);
        while(!todoNode.empty()) {
            KdTreeNode* node = todoNode.top();
            todoNode.pop();

            if (node->isLeaf) {
                Hitpoint hptemp;
                if (node->triangle.tri.intersect(ray, &hptemp)) {
                    if (hitpoint->distance() > hptemp.distance()) {
                        *hitpoint = hptemp;
                        tid = node->triangle.idx;
                    }
                }
            } else {
                double tmin, tmax;

                // Check which child is nearer
                if (node->left != nullptr) {
                    bool isectL = node->left->bbox.intersect(ray, &tmin, &tmax);
                    if (isectL) todoNode.push(node->left);
                }

                if (node->right != nullptr) {
                    bool isectR = node->right->bbox.intersect(ray, &tmin, &tmax);
                    if (isectR) todoNode.push(node->right);
                }
            }
        }
        return tid;
    }

}  // namespace spica

