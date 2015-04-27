#define SPICA_KDTREE_EXPORT
#include "kd_tree.h"

#include <algorithm>

namespace spica {

    KdTree::KdTree() 
        : _triangles(0)
    {
    }

    KdTree::~KdTree() 
    {
        delete[] _triangles;
    }

    void KdTree::construct() {
        // Sort bounding boxes
    }

    void KdTree::constructRec(int startID, int endID, int dim) {
        // Sort triangles
        // std::sort(_triangles + startID, _triangles + endID, AxisComparator(dim));

    }

}  // namespace spica

