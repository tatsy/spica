#ifndef _SPICA_KDTREE_H_
#define _SPICA_KDTREE_H_

#include <vector>
#include <queue>
#include <functional>

namespace spica {

    enum KnnSearchType {
        EPSILON_BALL = 0x01,
        K_NEAREST    = 0x02
    };

    struct KnnQuery {
        KnnSearchType type;
        int k;
        double epsilon;

        KnnQuery(int type_, int k_, double epsilon_)
            : type(static_cast<KnnSearchType>(type_))
            , k(k_)
            , epsilon(epsilon_)
        {
        }
    };

    template <class Ty>
    class KdTree {
    public:
        KdTree();
        KdTree(const KdTree<Ty>& kdtree);
        ~KdTree();

        KdTree& operator=(const KdTree<Ty>& kdtree);

        void add(const Ty& point);
        void construct(const std::vector<Ty>& points);
        void knnSearch(const Ty& point, const KnnQuery& query, std::vector<Ty>* results) const;

        void release();

    private:
        // Private internal classes
        struct OrderedType {
            double dist;
            Ty t;
            OrderedType(const double dist_, const Ty& t_)
            : dist(dist_)
            , t(t_)
            {
            }
            
            bool operator<(const OrderedType& t) const {
                return this->dist < t.dist;
            }
            bool operator>(const OrderedType& t) const {
                return this->dist > t.dist;
            }
        };
        
        typedef std::priority_queue<OrderedType, std::vector<OrderedType>, std::less<OrderedType> > PriorityQueue;
        
        struct KdTreeNode {
            Ty point;
            KdTreeNode* left;
            KdTreeNode* right;
            int axis;
            
            KdTreeNode()
            : point()
            , left(NULL)
            , right(NULL)
            , axis(0)
            {
            }
        };
        
        struct AxisComparator {
            int dim;
            explicit AxisComparator(int d) : dim(d) {}
            bool operator()(const Ty* t1, const Ty* t2) {
                return (*t1)[dim] < (*t2)[dim];
            }
        };
        
        // Private methods
        KdTreeNode* constructRec(std::vector<const Ty*> &points, int nodeID, int startID, int endID, int dim);
        KdTreeNode* addRec(KdTreeNode* node, const Ty &point, int dim);

        static double squaredDistance(const Ty& p1, const Ty& p2);

        // Private fields
        KdTreeNode* nodes_;
        int* numCopies_;
    };
}

#include "kdtree_detail.h"

#endif  // _SPICA_KDTREE_H_
