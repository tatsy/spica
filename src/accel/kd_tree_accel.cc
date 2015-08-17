#define SPICA_KDTREE_ACCEL_EXPORT
#include "kd_tree_accel.h"

#include <cstring>
#include <algorithm>

namespace spica {

    KdTreeAccel::KdTreeAccel() 
        : _root(NULL)
    {
    }

    KdTreeAccel::KdTreeAccel(const KdTreeAccel& kdtree)
        : _root(NULL)
    {
        this->operator=(kdtree);
    }

    KdTreeAccel::KdTreeAccel(KdTreeAccel&& kdtree)
        : _root(NULL)
    {
        this->operator=(std::move(kdtree));
    }

    KdTreeAccel::~KdTreeAccel() 
    {
        release();
    }

    KdTreeAccel& KdTreeAccel::operator=(const KdTreeAccel& kdtree) {
        release();

        _root = copyNode(kdtree._root);

        return *this;
    }

    KdTreeAccel& KdTreeAccel::operator=(KdTreeAccel&& kdtree) {
        release();

        _root = kdtree._root;
        kdtree._root = nullptr;

        return *this;
    }

    void KdTreeAccel::release() {
        deleteNode(_root);
    }

    void KdTreeAccel::deleteNode(KdTreeNode* node) {
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

    KdTreeAccel::KdTreeNode* KdTreeAccel::copyNode(KdTreeNode* node) {
        KdTreeNode* ret = NULL;
        if (node != NULL) {
            ret = new KdTreeNode();
            ret->bbox = node->bbox;
            ret->numTriangles = node->numTriangles;
            ret->triangles = new Triangle[node->numTriangles];
            memcpy((void*)ret->triangles, (void*)node->triangles, sizeof(Triangle) * node->numTriangles);
            ret->isLeaf = node->isLeaf;

            node->left = copyNode(node->left);
            node->right = copyNode(node->right);
        }
        return ret;
    }

    void KdTreeAccel::construct(const std::vector<Triangle>& triangles) {
        release();

        std::vector<Triangle> temp(triangles);
        _root = constructRec(temp, 0);
    }

    KdTreeAccel::KdTreeNode* KdTreeAccel::constructRec(std::vector<Triangle>& triangles, int dim)  {
        const int nTri = (int)triangles.size();

        // Sort triangles
        std::sort(triangles.begin(), triangles.end(), AxisComparator(dim));

        const int mid = nTri / 2;
        std::vector<Triangle> triLeft(triangles.begin(), triangles.begin() + mid);
        std::vector<Triangle> triRight(triangles.begin() + mid, triangles.end());

        if (triangles.size() <= _maxNodeSize || (triLeft.size() == nTri || triRight.size() == nTri)) {
            KdTreeNode* node = new KdTreeNode();
            node->numTriangles = nTri;
            node->triangles = new Triangle[nTri];
            memcpy((void*)node->triangles, (void*)&triangles[0], sizeof(Triangle)* nTri);
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

    int KdTreeAccel::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        double tMin, tMax;
        KdTreeNode* node = _root;
        if (!node->bbox.intersect(ray, &tMin, &tMax)) {
            return -1;
        }

        return intersectRec(node, ray, hitpoint, tMin, tMax);
    }

    int KdTreeAccel::intersectRec(KdTreeNode* node, const Ray& ray, Hitpoint* hitpoint, double tMin, double tMax) {
        if (node->isLeaf) {
            int triID = -1;
            for (int i = 0; i < node->numTriangles; i++) {
                const Triangle& tri = node->triangles[i];
                Hitpoint hpTemp;
                if (tri.intersect(ray, &hpTemp)) {
                    if (hitpoint->distance() > hpTemp.distance() && Vector3D::dot(ray.direction(), tri.normal()) < 0.0) {
                        *hitpoint = hpTemp;
                        triID = i;
                    }
                }
            }
            return triID;
        }

        // Check which child is nearer
        double lMin = INFTY, lMax = INFTY, rMin = INFTY, rMax = INFTY;
        bool isectL = node->left->bbox.intersect(ray, &lMin, &lMax);
        bool isectR = node->right->bbox.intersect(ray, &rMin, &rMax);

        // Intesecting NO children
        if (!isectL && !isectR) {
            return -1;
        }

        // Intersecting only one child
        if (isectL && !isectR) {
            return intersectRec(node->left, ray, hitpoint, lMin, lMax);
        }

        if (isectR && !isectL) {
            return intersectRec(node->right, ray, hitpoint, rMin, rMax);
        }

        // Intersecting two children
        KdTreeNode *nearer, *farther;
        if (lMin < rMin || (lMin == rMin && lMax == INFTY)) {
            nearer = node->left;
            farther = node->right;
        } else {
            nearer = node->right;
            farther = node->left;
            std::swap(lMin, rMin);
            std::swap(lMax, rMax);
        }

        // Check nearer child first
        const int triID = intersectRec(nearer, ray, hitpoint, lMin, lMax);
        if (triID != -1) {
            return triID;
        }
        return intersectRec(farther, ray, hitpoint, rMin, rMax);
    }

}  // namespace spica

