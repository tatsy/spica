#define SPICA_KDTREE_EXPORT
#include "kd_tree.h"

#include <cstring>
#include <algorithm>

namespace spica {

    KdTree::KdTree() 
        : _root(NULL)
        , _numCopies(NULL)
    {
    }

    KdTree::KdTree(const KdTree& kdtree)
        : _root(NULL)
        , _numCopies(NULL)
    {
        operator=(kdtree);
    }

    KdTree::~KdTree() 
    {
        release();
    }

    KdTree& KdTree::operator=(const KdTree& kdtree) {
        release();

        _root = kdtree._root;
        _numCopies = kdtree._numCopies;
        (*_numCopies) += 1;

        return *this;
    }

    void KdTree::release() {
        if (_numCopies == 0) {
            deleteNode(_root);
            delete _numCopies;
        } else {
            _numCopies--;
        }
    }

    void KdTree::deleteNode(KdTreeNode* node) {
        if (node != NULL) {
            if (node->left != NULL) {
                deleteNode(node->left);
            }

            if (node->right != NULL) {
                deleteNode(node->right);
            }

            delete node;
        }
    }

    void KdTree::construct(std::vector<Triangle>& triangles) {
        release();

        _root = constructRec(triangles, 0);
        _numCopies = new unsigned int(0);
    }

    BBox KdTree::enclosingBox(const std::vector<Triangle>& triangles) {
        Vector3 posMin(INFTY, INFTY, INFTY);
        Vector3 posMax(-INFTY, -INFTY, -INFTY);
        const int nTri = (int)triangles.size();
        for (int i = 0; i < nTri; i++) {
            for (int j = 0; j < 3; j++) {
                posMin = Vector3::minimum(posMin, triangles[i].p(j));
                posMax = Vector3::maximum(posMax, triangles[i].p(j));
            }
        }
        return BBox(posMin, posMax);
    }

    KdTreeNode* KdTree::constructRec(std::vector<Triangle>& triangles, int dim)  {
        const int nTri = (int)triangles.size();

        // Sort triangles
        std::sort(triangles.begin(), triangles.end(), AxisComparator(dim));
        double sep = triangles[nTri / 2].gravity().get(dim);

        std::vector<Triangle> triLeft;
        for (int i = 0; i < nTri; i++) {
            bool inc = false;
            for (int k = 0; k < 3; k++) {
                if (triangles[i].p(k).get(dim) <= sep) {
                    inc = true;
                    break;
                }
            }

            if (inc) {
                triLeft.push_back(triangles[i]);
            }
        }

        std::vector<Triangle> triRight;
        for (int i = 0; i < nTri; i++) {
            bool inc = false;
            for (int k = 0; k < 3; k++) {
                if (triangles[i].p(k).get(dim) >= sep) {
                    inc = true;
                    break;
                }
            }

            if (inc) {
                triRight.push_back(triangles[i]);
            }
        }

        if (triangles.size() <= _maxNodeSize || (triLeft.size() == nTri || triRight.size() == nTri)) {
            KdTreeNode* node = new KdTreeNode();
            node->numTriangles = nTri;
            node->triangles = new Triangle[nTri];
            memcpy(node->triangles, &triangles[0], sizeof(Triangle)* nTri);
            node->bbox = enclosingBox(triangles);
            node->left = NULL;
            node->right = NULL;
            node->isLeaf = true;
            return node;
        }

        KdTreeNode* node = new KdTreeNode();
        node->numTriangles = 0;
        node->triangles = NULL;
        node->bbox = enclosingBox(triangles);
        node->left = constructRec(triLeft, (dim + 1) % 3);
        node->right = constructRec(triRight, (dim + 1) % 3);
        node->isLeaf = false;

        return node;
    }

}  // namespace spica

