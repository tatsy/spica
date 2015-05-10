#ifndef _SPICA_KDTREE_ACCEL_H_
#define _SPICA_KDTREE_ACCEL_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_KDTREE_ACCEL_EXPORT
        #define SPICA_KDTREE_ACCEL_DLL __declspec(dllexport)
    #else
        #define SPICA_KDTREE_ACCEL_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_KDTREE_ACCEL_DLL 
#endif

#include "accel_base.h"
#include "../utils/common.h"
#include "../utils/uncopyable.h"
#include "../geometry/bbox.h"

namespace spica {

    struct KdTreeNode : public Uncopyable {
        BBox bbox;
        unsigned int numTriangles;
        Triangle* triangles;
        KdTreeNode* left;
        KdTreeNode* right;
        bool isLeaf;

        KdTreeNode()
            : bbox()
            , numTriangles(0)
            , triangles(NULL)
            , left(NULL)
            , right(NULL)
            , isLeaf(false)
        {
        }

        ~KdTreeNode()
        {
            delete[] triangles;
        }
    };

    class SPICA_KDTREE_ACCEL_DLL KdTreeAccel : public AccelBase {
    private:

        // ----------------------------------------------
        // Comparator to sort triangles 
        // ----------------------------------------------
        struct AxisComparator {
            int dim;
            explicit AxisComparator(int dim_ = 0)
                : dim(dim_)
            {
                msg_assert(0 <= dim_ && dim_ <= 2, "Dimension must be between 0 and 2");
            }

            bool operator()(const Triangle& t1, const Triangle& t2) const {
                return t1.gravity().get(dim) < t2.gravity().get(dim);
            }
        };

        static const int _maxNodeSize = 5;
        KdTreeNode* _root;          // tree root
        unsigned int* _numCopies;   // # of tree copies

    public:
        KdTreeAccel();
        KdTreeAccel(const KdTreeAccel& kdtree);
        ~KdTreeAccel();

        KdTreeAccel& operator=(const KdTreeAccel& kdtree);
        
        void construct(const std::vector<Triangle>& triangles);

        inline bool empty() const { return _root == 0; }

        inline KdTreeNode* root() const { return _root; }

    private:
        void release();
        void deleteNode(KdTreeNode* node);
        KdTreeNode* constructRec(std::vector<Triangle>& triangles, int dim);

        static BBox enclosingBox(const std::vector<Triangle>& triangles);
    };

}

#endif  // _SPICA_KDTREE_H_
