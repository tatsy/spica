#define SPICA_KDTREE_EXPORT
#include "kd_tree.h"

#include <cstring>
#include <algorithm>

namespace spica {

    KdTree::KdTree() 
        : _numTriangles(0)
        , _numNodes(0)
        , _nodes(0)
        , _triangles(0)
    {
    }

    KdTree::KdTree(const KdTree& kdtree)
        : _numTriangles(0)
        , _numNodes(0)
        , _nodes(0)
        , _triangles(0)
    {
        operator=(kdtree);
    }

    KdTree::~KdTree() 
    {
        release();
    }

    KdTree& KdTree::operator=(const KdTree& kdtree) {
        release();
        _numTriangles = kdtree._numTriangles;
        _numNodes = kdtree._numNodes;
        _triangles = new Triangle[_numTriangles];
        _nodes = new KdTreeNode[_numNodes];
        memcpy(_triangles, kdtree._triangles, sizeof(Triangle)* _numTriangles);
        memcpy(_nodes, kdtree._nodes, sizeof(KdTreeNode) * _numNodes);
        return *this;
    }

    void KdTree::release() {
        _numTriangles = 0;
        _numNodes = 0;
        delete[] _nodes;
        delete[] _triangles;        
    }

    void KdTree::construct(const std::vector<Triangle>& triangles) {
        release();

        // Sort bounding boxes
        _numTriangles = (int)triangles.size();
        for (_numNodes = 1; _numNodes < _numTriangles * 2; _numNodes <<= 1) ;
        
        _nodes = new KdTreeNode[_numNodes];
        _triangles = new Triangle[_numTriangles];

        // Copy triangles
        memcpy(_triangles, &triangles[0], sizeof(Triangle) * _numTriangles);

        constructRec(0, 0, _numTriangles, 0);
    }

    BBox KdTree::enclosingBox(int startID, int endID) const {
        Vector3 posMin(INFTY, INFTY, INFTY);
        Vector3 posMax(-INFTY, -INFTY, -INFTY);
        for (int i = startID; i < endID; i++) {
            for (int j = 0; j < 3; j++) {
                posMin = Vector3::minimum(posMin, _triangles[i].p(j));
                posMax = Vector3::maximum(posMax, _triangles[i].p(j));
            }
        }
        return BBox(posMin, posMax);
    }

    KdTreeNode* KdTree::constructRec(int nodeID, int startID, int endID, int dim) {
        if (startID + 1 >= endID) {
            return NULL;
        }

        // Sort triangles
        std::sort(_triangles + startID, _triangles + endID, AxisComparator(dim));

        const int mid = (startID + endID) / 2;
        _nodes[nodeID].startID = startID;
        _nodes[nodeID].endID = endID;
        _nodes[nodeID].bbox = enclosingBox(startID, endID);
        _nodes[nodeID].left = constructRec(nodeID * 2 + 1, startID, mid, (dim + 1) % 3);
        _nodes[nodeID].right = constructRec(nodeID * 2 + 2, mid, endID, (dim + 1) % 3);

        return &_nodes[nodeID];
    }

}  // namespace spica

