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
        double epsilon;
        int k;
        KnnQuery(int type_, double epsilon_, int k_)
            : type(static_cast<KnnSearchType>(type_))
            , epsilon(epsilon_)
            , k(k_)
        {
        }
    };

    template <class Ty>
    class KdTree {
    private:
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
        };

        struct AxisComparator {
            int dim;
            explicit AxisComparator(int d) : dim(d) {}
            bool operator()(const Ty& t1, const Ty& t2) {
                return t1.get(dim) < t2.get(dim);
            }
        };

        KdTreeNode* _root;
        int* _numCopies;

    public:
        KdTree();
        KdTree(const KdTree<Ty>& kdtree);
        ~KdTree();

        KdTree& operator=(const KdTree<Ty>& kdtree);

        void construct(const std::vector<Ty>& points);
        void knnSearch(const Ty& point, const KnnQuery& query, std::vector<Ty>* results);

        void release();

    private:
        void deleteNode(KdTreeNode* node);

        KdTreeNode* constructRec(std::vector<Ty>& points, int startID, int endID, int dim);
        void knnSearchRec(KdTreeNode* node, const Ty& point, KnnQuery& query, PriorityQueue* results);
    };
}

#include "kdtree_detail.h"

#endif  // _SPICA_KDTREE_H_