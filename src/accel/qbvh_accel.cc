#define SPICA_QBVH_ACCEL_EXPORT
#include "qbvh_accel.h"

#include <stack>
#include <cmath>
#include <cstring>
#include <algorithm>

#include "../utils/common.h"

namespace spica {

    namespace {
    
        const float inff = (float)1.0e20;
        align_attrib(float, 16) infs[4]  = { inff, inff, inff, inff };
        align_attrib(float, 16) ninfs[4] = { -inff, -inff, -inff, -inff };
        align_attrib(float, 16) zeros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        __m128 simdInf = _mm_load_ps(infs);
        __m128 simdNinf = _mm_load_ps(ninfs);
        __m128 simdZero = _mm_load_ps(zeros);

        static const int orderTable[] = {
          //+++      -++      +-+      --+      ++-      -+-      +--      ---       <-- right, left, top
            0x44444, 0x44444, 0x44444, 0x44444, 0x44444, 0x44444, 0x44444, 0x44444,  // --|-- (TL, TR | BL, BR)
            0x44440, 0x44440, 0x44440, 0x44440, 0x44440, 0x44440, 0x44440, 0x44440,  // +-|--
            0x44441, 0x44441, 0x44441, 0x44441, 0x44441, 0x44441, 0x44441, 0x44441,  // -+|--
            0x44401, 0x44401, 0x44410, 0x44410, 0x44401, 0x44401, 0x44410, 0x44410,  // ++|--
            0x44442, 0x44442, 0x44442, 0x44442, 0x44442, 0x44442, 0x44442, 0x44442,  // --|+-
            0x44402, 0x44402, 0x44402, 0x44402, 0x44420, 0x44420, 0x44420, 0x44420,  // +-|+-
            0x44412, 0x44412, 0x44412, 0x44412, 0x44421, 0x44421, 0x44421, 0x44421,  // -+|+-
            0x44012, 0x44012, 0x44102, 0x44102, 0x44201, 0x44201, 0x44210, 0x44210,  // ++|+-
            0x44443, 0x44443, 0x44443, 0x44443, 0x44443, 0x44443, 0x44443, 0x44443,  // --|-+
            0x44403, 0x44403, 0x44403, 0x44403, 0x44430, 0x44430, 0x44430, 0x44430,  // +-|-+
            0x44413, 0x44413, 0x44413, 0x44413, 0x44431, 0x44431, 0x44431, 0x44431,  // -+|-+
            0x44013, 0x44013, 0x44103, 0x44103, 0x44301, 0x44301, 0x44310, 0x44310,  // ++|-+
            0x44423, 0x44432, 0x44423, 0x44432, 0x44423, 0x44432, 0x44423, 0x44432,  // --|++
            0x44023, 0x44032, 0x44023, 0x44032, 0x44230, 0x44320, 0x44230, 0x44320,  // +-|++
            0x44123, 0x44132, 0x44123, 0x44132, 0x44231, 0x44321, 0x44231, 0x44321,  // -+|++
            0x40123, 0x40132, 0x41023, 0x41032, 0x42301, 0x43201, 0x42310, 0x43210,  // ++|++
        };

    }

    QBVHAccel::QBVHAccel()
        : _root(NULL)
    {
    }

    QBVHAccel::QBVHAccel(const QBVHAccel& qbvh)
        : _root(NULL)
    {
        this->operator=(qbvh);
    }

    QBVHAccel::QBVHAccel(QBVHAccel&& qbvh) 
        : _root(NULL)
    {
        this->operator=(std::move(qbvh));    
    }

    QBVHAccel::~QBVHAccel()
    {
        release();
    }

    QBVHAccel& QBVHAccel::operator=(const QBVHAccel& qbvh) {
        release();

        _root = copyNode(qbvh._root);

        return *this;
    }

    QBVHAccel& QBVHAccel::operator=(QBVHAccel&& qbvh) {
        release();

        _root = qbvh._root;
        qbvh._root = nullptr;

        return *this;
    }

    void QBVHAccel::release() {
        deleteNode(_root);
    }

    void QBVHAccel::deleteNode(QBVHNode* node) {
        if (node != NULL) {
            for (int i = 0; i < 4; i++) {
                deleteNode(node->children[i]);
            }
            delete node;
            node = NULL;
        }
    }

    QBVHAccel::QBVHNode* QBVHAccel::copyNode(QBVHNode* node) {
        QBVHNode* ret = NULL;
        if (node != NULL) {
            ret = new QBVHNode();
            memcpy((void*)ret->childBoxes, (void*)node->childBoxes, sizeof(__m128) * 6);
            ret->triangles.resize(node->triangles.size());
            std::copy(node->triangles.begin(), node->triangles.end(), ret->triangles.begin());
            memcpy((void*)ret->sepAxes, (void*)node->sepAxes, sizeof(char) * 3);
            ret->isLeaf = node->isLeaf;

            for (int i = 0; i < 4; i++) {
                ret->children[i] = copyNode(node->children[i]);
            }
        }
        return ret;
    }

    void QBVHAccel::construct(const std::vector<Triangle>& triangles) {
        release();

        const int numTriangles = static_cast<int>(triangles.size());
        std::vector<TriangleWithID> temp(numTriangles);
        for (int i = 0; i < triangles.size(); i++) {
            temp[i].first = triangles[i];
            temp[i].second = i;
        }
        _root = constructRec(temp, 0);
    }
    
    QBVHAccel::QBVHNode* QBVHAccel::constructRec(std::vector<TriangleWithID>& triangles, int dim) {
        const int nTri = static_cast<int>(triangles.size());

        if (triangles.size() <= _maxNodeSize) {
            QBVHNode* node = new QBVHNode();
            node->triangles.resize(nTri);
            std::copy(triangles.begin(), triangles.end(), node->triangles.begin());
            node->isLeaf = true;
            return node;
        }

        std::sort(triangles.begin(), triangles.end(), AxisComparator(dim));
        
        const int mid = nTri / 2;
        std::sort(triangles.begin(), triangles.begin() + mid, AxisComparator((dim + 1) % 3));
        std::sort(triangles.begin() + mid, triangles.end(), AxisComparator((dim + 1) % 3));

        std::vector<TriangleWithID> c0(triangles.begin(), triangles.begin() + (mid / 2));
        std::vector<TriangleWithID> c1(triangles.begin() + (mid / 2), triangles.begin() + mid);
        std::vector<TriangleWithID> c2(triangles.begin() + mid, triangles.begin() + (mid + mid / 2));
        std::vector<TriangleWithID> c3(triangles.begin() + (mid + mid / 2), triangles.end());

        BBox boxes[4];
        boxes[0] = enclosingBox(c0);
        boxes[1] = enclosingBox(c1);
        boxes[2] = enclosingBox(c2);
        boxes[3] = enclosingBox(c3);

        QBVHNode* node = new QBVHNode();
        align_attrib(float, 16) cboxes[2][3][4];
        for (int i = 0; i < 4; i++) {
            for (int d = 0; d < 3; d++) {
                cboxes[0][d][i] = static_cast<float>(boxes[i].posMin().get(d));  
                cboxes[1][d][i] = static_cast<float>(boxes[i].posMax().get(d));
            }
        }

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 3; j++) {
                node->childBoxes[i][j] = _mm_load_ps(cboxes[i][j]);
            }
        }

        node->children[0] = constructRec(c0, (dim + 2) % 3);
        node->children[1] = constructRec(c1, (dim + 2) % 3);
        node->children[2] = constructRec(c2, (dim + 2) % 3);
        node->children[3] = constructRec(c3, (dim + 2) % 3);
        node->sepAxes[0] = (char)dim;
        node->sepAxes[1] = (char)((dim + 1) % 3);
        node->sepAxes[2] = (char)((dim + 1) % 3);
        node->isLeaf = false;
        return node;
    }

    int QBVHAccel::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        // ray for SIMD arthimetic
        __m128 simdOrig[3];  // origin
        __m128 simdIdir[3];  // inverse direction
        int sgn[3];          // signs of ray direction (pos -> 0, neg -> 1)
        
        float orgx = static_cast<float>(ray.origin().x());
        float orgy = static_cast<float>(ray.origin().y());
        float orgz = static_cast<float>(ray.origin().z());

        align_attrib(float, 16) orgxs[4] = { orgx, orgx, orgx, orgx };
        align_attrib(float, 16) orgys[4] = { orgy, orgy, orgy, orgy };
        align_attrib(float, 16) orgzs[4] = { orgz, orgz, orgz, orgz };

        float idirx = ray.direction().x() == 0.0 ? (float)1.0e20 : (float)(1.0 / ray.direction().x());
        float idiry = ray.direction().y() == 0.0 ? (float)1.0e20 : (float)(1.0 / ray.direction().y());
        float idirz = ray.direction().z() == 0.0 ? (float)1.0e20 : (float)(1.0 / ray.direction().z());

        align_attrib(float, 16) idirxs[4] = { idirx, idirx, idirx, idirx };
        align_attrib(float, 16) idirys[4] = { idiry, idiry, idiry, idiry };
        align_attrib(float, 16) idirzs[4] = { idirz, idirz, idirz, idirz };

        simdOrig[0] = _mm_load_ps(orgxs);
        simdOrig[1] = _mm_load_ps(orgys);
        simdOrig[2] = _mm_load_ps(orgzs);

        simdIdir[0] = _mm_load_ps(idirxs);
        simdIdir[1] = _mm_load_ps(idirys);
        simdIdir[2] = _mm_load_ps(idirzs);

        sgn[0] = idirx > 0.0f ? 0 : 1;
        sgn[1] = idiry > 0.0f ? 0 : 1;
        sgn[2] = idirz > 0.0f ? 0 : 1;

        int hit = -1;
        std::stack<QBVHNode*> stk;
        stk.push(_root);
        while(!stk.empty()) {
            QBVHNode* node = stk.top();
            stk.pop();

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

                if (triID != -1) {
                    hit = triID;
                }
                continue;
            }

            // Test ray-bbox intersection
            float hitdist = static_cast<float>(hitpoint->distance());
            align_attrib(float, 16) hitdists[4] = { hitdist, hitdist, hitdist, hitdist };
            __m128 tMin = simdZero;
            __m128 tMax = _mm_load_ps(hitdists);

            for (int d = 0; d < 3; d++) {
                tMin = _mm_max_ps(tMin, _mm_mul_ps(_mm_sub_ps(node->childBoxes[sgn[d]][d], simdOrig[d]), simdIdir[d]));
                tMax = _mm_min_ps(tMax, _mm_mul_ps(_mm_sub_ps(node->childBoxes[1 - sgn[d]][d], simdOrig[d]), simdIdir[d]));
            }

            int hitMask = _mm_movemask_ps(_mm_cmpge_ps(tMax, tMin));
            if (hitMask != 0) {
                int sepMask = (sgn[node->sepAxes[2]] << 2) | (sgn[node->sepAxes[1]] << 1) | (sgn[node->sepAxes[0]]);
                int ordMask = orderTable[hitMask * 8 + sepMask];
                for (int i = 0; i < 4; i++) {    
                    if (ordMask & 0x04) break;
                    stk.push(node->children[ordMask & 0x03]);
                    ordMask >>= 4;
                }
            }
        }

        return hit;
    }
}