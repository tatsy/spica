#ifndef _SPICA_KD_TREE_H_
#define _SPICA_KD_TREE_H_

namespace spica {

    class KdTree {

        struct KdTreeNode {
            int startID;
            int endID;
            KdTreeNode* left;
            KdTreeNode* right;
        };

    };

}

#endif  // _SPICA_KD_TREE_H_
