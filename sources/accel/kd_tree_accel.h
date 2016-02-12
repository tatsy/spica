#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_KDTREE_ACCEL_H_
#define _SPICA_KDTREE_ACCEL_H_

#include <vector>

#include "accel_interface.h"
#include "../core/common.h"
#include "../core/uncopyable.h"

namespace spica {

/**
 * K-D tree accelerator class
 * @ingroup accel_module
 */
class SPICA_EXPORTS KdTreeAccel : public AccelInterface {
private:
    struct KdTreeNode {
        Bound3d bbox;
        IndexedTriangle triangle;
        KdTreeNode* left;
        KdTreeNode* right;
        bool isLeaf;

        KdTreeNode()
            : bbox()
            , triangle()
            , left(nullptr)
            , right(nullptr)
            , isLeaf(false) {
        }

        ~KdTreeNode() {
        }
    };

    KdTreeNode* _root;          // tree root

public:
    KdTreeAccel();
    ~KdTreeAccel();
        
    Bound3d worldBound() const override;
    void construct(const std::vector<Triangle>& triangles) override;
    bool intersect(const Ray& ray, SurfaceInteraction* isect) const override;

private:
    void release();
    void deleteNode(KdTreeNode* node);
    KdTreeNode* copyNode(KdTreeNode* node);
    KdTreeNode* constructRec(std::vector<IndexedTriangle>& triangles, int start, int end);
};

}  // namespace spica

#endif  // _SPICA_KDTREE_H_
