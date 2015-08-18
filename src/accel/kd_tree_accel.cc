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
            ret->triangles.resize(node->triangles.size());
            std::copy(node->triangles.begin(), node->triangles.end(), ret->triangles.begin());
            ret->isLeaf = node->isLeaf;

            node->left = copyNode(node->left);
            node->right = copyNode(node->right);
        }
        return ret;
    }

    void KdTreeAccel::construct(const std::vector<Triangle>& triangles) {
        release();

        const int numTriangles = static_cast<int>(triangles.size());
        std::vector<TriangleWithID> temp(numTriangles);
        for (int i = 0; i < numTriangles; i++) {
            temp[i].first = triangles[i];
            temp[i].second = i;
        }
        _root = constructRec(temp, 0);
    }

    KdTreeAccel::KdTreeNode* KdTreeAccel::constructRec(std::vector<TriangleWithID>& triangles, int dim) {
        const int nTri = static_cast<int>(triangles.size());

        // Sort triangles
        std::sort(triangles.begin(), triangles.end(), AxisComparator(dim));

        const int mid = nTri / 2;
        std::vector<TriangleWithID> triLeft(triangles.begin(), triangles.begin() + mid);
        std::vector<TriangleWithID> triRight(triangles.begin() + mid, triangles.end());

        if (triangles.size() <= _maxNodeSize || (triLeft.size() == nTri || triRight.size() == nTri)) {
            KdTreeNode* node = new KdTreeNode();
            node->triangles.resize(nTri);
            std::copy(triangles.begin(), triangles.end(), node->triangles.begin());
            node->bbox = enclosingBox(triangles);
            node->left = NULL;
            node->right = NULL;
            node->isLeaf = true;
            return node;
        }

        KdTreeNode* node = new KdTreeNode();
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
            for (int i = 0; i < node->triangles.size(); i++) {
                const Triangle& tri = node->triangles[i].first;
                Hitpoint hpTemp;
                if (tri.intersect(ray, &hpTemp)) {
                    if (hitpoint->distance() > hpTemp.distance() && Vector3D::dot(ray.direction(), tri.normal()) < 0.0) {
                        *hitpoint = hpTemp;
                        triID = node->triangles[i].second;
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
        Hitpoint lefthp, righthp;
        const int trileft = intersectRec(node->left, ray, &lefthp, lMin, lMax);
        const int triright = intersectRec(node->right, ray, &righthp, rMin, rMax);
        if (trileft != -1 && triright != -1) {
            if (lefthp.distance() < righthp.distance()) {
                *hitpoint = lefthp;
                return trileft;
            } else {
                *hitpoint = righthp;
                return triright;
            }
        }

        if (trileft != -1) {
            *hitpoint = lefthp;
            return trileft;
        }

        if (triright != -1) {
            *hitpoint = righthp;
            return triright;
        }

        return -1;
    }

}  // namespace spica

