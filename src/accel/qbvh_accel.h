#ifndef _SPICA_QBVH_ACCEL_H_
#define _SPICA_QBVH_ACCEL_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_QBVH_ACCEL_EXPORT
        #define SPICA_QBVH_ACCEL_DLL __declspec(dllexport)
    #else
        #define SPICA_QBVH_ACCEL_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_QBVH_ACCEL_DLL
#endif

#include <vector>
#include <xmmintrin.h>

#include "../utils/uncopyable.h"
#include "accel_base.h"

namespace spica {

    class QBVHAccel : public AccelBase {
    private:
        struct QBVHNode : public Uncopyable {
            __m128 childBoxes[2][3];  // [min-max][x-y-z]
            QBVHNode* children[4];    // Child nodes
            std::vector<TriangleWithID> triangles;
            char sepAxes[3];          // top-left-right
            bool isLeaf;

            QBVHNode()
                : childBoxes()
                , children()
                , triangles()
                , sepAxes()
                , isLeaf(false)
            {
            }

            ~QBVHNode()
            {
            }
        };

        static const int _maxNodeSize = 3;
        QBVHNode* _root;

    public:
        QBVHAccel();
        QBVHAccel(const QBVHAccel& qbvh);
        QBVHAccel(QBVHAccel&& qbvh);
        ~QBVHAccel();

        QBVHAccel& operator=(const QBVHAccel& qbvh);
        QBVHAccel& operator=(QBVHAccel&& qbvh);

        void construct(const std::vector<Triangle>& triangles);

        int intersect(const Ray& ray, Hitpoint* hitpoint) const override;

    private:
        void release();
        void deleteNode(QBVHNode* node);
        QBVHNode* copyNode(QBVHNode* node);
        
        QBVHNode* constructRec(std::vector<TriangleWithID>& triangles, int dim);
    };

}

#endif  // _SPICA_QBVH_ACCEL_H_

