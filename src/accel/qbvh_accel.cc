#define SPICA_QBVH_ACCEL_EXPORT
#include "qbvh_accel.h"

#include <stack>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "../utils/common.h"

namespace spica {

    namespace {
    
        const float inff = (float)INFTY;
        const float epsf = (float)EPS;
        align_attrib(float, 16) infs[4]  = { inff, inff, inff, inff };
        align_attrib(float, 16) zeros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        align_attrib(float, 16) epss[4] = { epsf, epsf, epsf, epsf };
        __m128 simdInf = _mm_load_ps(infs);
        __m128 simdZero = _mm_load_ps(zeros);
        __m128 simdEps  = _mm_load_ps(epss);

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

        struct BucketInfo {
            int count;
            BBox bounds;
            BucketInfo() : count(0), bounds() { }
        };

        int test_AABB(const __m128 bboxes[2][3], const __m128 org[3], const __m128 idir[3], const int sign[3], __m128 tmin, __m128 tmax) {
            tmin = _mm_max_ps(tmin, _mm_mul_ps(_mm_sub_ps(bboxes[sign[0]][0], org[0]), idir[0]));
            tmax = _mm_min_ps(tmax, _mm_mul_ps(_mm_sub_ps(bboxes[1 - sign[0]][0], org[0]), idir[0]));
            tmin = _mm_max_ps(tmin, _mm_mul_ps(_mm_sub_ps(bboxes[sign[1]][1], org[1]), idir[1]));
            tmax = _mm_min_ps(tmax, _mm_mul_ps(_mm_sub_ps(bboxes[1 - sign[1]][1], org[1]), idir[1]));
            tmin = _mm_max_ps(tmin, _mm_mul_ps(_mm_sub_ps(bboxes[sign[2]][2], org[2]), idir[2]));
            tmax = _mm_min_ps(tmax, _mm_mul_ps(_mm_sub_ps(bboxes[1 - sign[2]][2], org[2]), idir[2]));
            return _mm_movemask_ps(_mm_cmpge_ps(tmax, tmin));
        }

    }  // anonymous namespace


    QBVHAccel::QBVHAccel()
        : _root(NULL)
        , _triangles()
        , _ordered()
        , _simdNodes()
        , _simdTris()
    {
    }

    QBVHAccel::~QBVHAccel()
    {
        release();
    }

    void QBVHAccel::release() {
        for (int i = 0; i < _simdNodes.size(); i++) {
            align_free(_simdNodes[i]);
        }

        for (int i = 0; i < _simdTris.size(); i++) {
            align_free(_simdTris[i]);
        }

        _root = nullptr;
        _ordered.clear();
        _triangles.clear();
        _simdNodes.clear();
        _simdTris.clear();
    }

    void QBVHAccel::construct(const std::vector<Triangle>& triangles) {
        // Destruct previos tree
        release();

        _triangles = triangles;

        std::vector<BVHPrimitiveInfo> buildData;
        for (int i = 0; i < triangles.size(); i++) {
            BBox b = BBox::fromTriangle(triangles[i]);
            buildData.emplace_back(i, b);
        }

        int totalNodes = 0;
        _ordered.reserve(triangles.size());

        _root = constructRec(buildData, 0, triangles.size(), &totalNodes, _ordered);

        collapse2QBVH(_root);
    }
    
    QBVHAccel::BVHBuildNode*
    QBVHAccel::constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                            int start, int end, int* totalNodes, 
                            std::vector<int>& orderedPrims) {
        (*totalNodes)++;
        BVHBuildNode* node = new BVHBuildNode();

        BBox bbox;
        for (int i = start; i < end; i++) {
            bbox.merge(buildData[i].bounds);
        }

        int nPrimitives = end - start;
        if (nPrimitives <= 4) {
            // This is leaf node
            int firstPrimOffset = orderedPrims.size();
            
            SIMDTrianglePack* simdt = (SIMDTrianglePack*)align_alloc(sizeof(SIMDTrianglePack), 16);
            Assertion(simdt != nullptr, "allocation failed !!");

            align_attrib(float, 16) x[4 * 3] = {0};
            align_attrib(float, 16) y[4 * 3] = {0};
            align_attrib(float, 16) z[4 * 3] = {0};

            int cnt = 0;
            for (int i = start; i < end; i++, cnt++) {
                const int idx = buildData[i].primitiveNumber;
                orderedPrims.push_back(idx);

                int t = cnt % 4;

                simdt->idx[t] = firstPrimOffset + cnt;
                for (int k = 0; k < 3; k++) {
                    x[4 * k + t] = _triangles[idx].get(k).x();
                    y[4 * k + t] = _triangles[idx].get(k).y();
                    z[4 * k + t] = _triangles[idx].get(k).z();
                }
            }

            for (; cnt < 4; cnt++) {
                simdt->idx[cnt % 4] = -1;
            }

            for (int i = 0; i < 3; i++) {
                simdt->x[i] = _mm_load_ps(x + 4 * i);
                simdt->y[i] = _mm_load_ps(y + 4 * i);
                simdt->z[i] = _mm_load_ps(z + 4 * i);
            }

            _simdTris.push_back(simdt);
            node->InitLeaf(firstPrimOffset, nPrimitives, bbox, _simdTris.size() - 1);
        } else {
            // This is fork node
            BBox centroidBounds;
            for (int i = start; i < end; i++) {
                centroidBounds.merge(buildData[i].centroid);
            }
            int dim = centroidBounds.maximumExtent();
            int mid = (start + end) / 2;

            if (nPrimitives <= 16) {
                std::nth_element(buildData.begin() + start,
                                 buildData.begin() + mid,
                                 buildData.begin() + end, ComparePoint(dim));
            } else {
                const int nBuckets = 12;
                BucketInfo buckets[nBuckets];
                for (int i = start; i < end; i++) {
                    int b = (int)(nBuckets * (buildData[i].centroid.get(dim) - centroidBounds.posMin().get(dim)) / 
                                 (centroidBounds.posMax().get(dim) - centroidBounds.posMin().get(dim)));
                    if (b == nBuckets) {
                        b = nBuckets - 1;
                    }
                    buckets[b].count++;
                    buckets[b].bounds.merge(buildData[i].bounds);
                }

                float cost[nBuckets - 1] = {0};
                for (int i = 0; i < nBuckets - 1; i++) {
                    BBox b0, b1;
                    int count0 = 0, count1 = 0;
                    for (int j = 0; j <= i; j++) {
                        b0.merge(buckets[j].bounds);
                        count0 += buckets[j].count;
                    }
                    for (int j = i + 1; j < nBuckets; j++) {
                        b1.merge(buckets[j].bounds);
                        count1 += buckets[j].count;
                    }
                    cost[i] += 0.125f + (count0 * b0.area() + count1 * b1.area()) / bbox.area();
                }

                float minCost = cost[0];
                int minCostSplit = 0;
                for (int i = 1; i < nBuckets - 1; i++) {
                    if (cost[i] < minCost) {
                        minCost = cost[i];
                        minCostSplit = i;
                    }
                }

                if (nPrimitives > 0 || minCost < nPrimitives) {
                    BVHPrimitiveInfo* pmid = std::partition(&buildData[start],
                                                            &buildData[end - 1] + 1, CompareToBucket(minCostSplit, nBuckets, dim, centroidBounds));
                    mid = pmid - &buildData[0];
                }
            }

            node->InitInterior(dim,
                constructRec(buildData, start, mid, totalNodes, orderedPrims),
                constructRec(buildData, mid, end, totalNodes, orderedPrims));
        }

        return node;
    }

    void QBVHAccel::collapse2QBVH(BVHBuildNode* node) {
        BVHBuildNode *lc = node->children[0];
        BVHBuildNode *rc = node->children[1];

        BVHBuildNode* c[4] = {0};

        SIMDBVHNode* n;
        n = (SIMDBVHNode*)align_alloc(sizeof(SIMDBVHNode), 16);
        Assertion(n != nullptr, "allocation failed !!");

        _simdNodes.push_back(n);
        n->axis_top = node->splitAxis;
        n->axis_left = n->axis_right = 0;

        if (lc != NULL) {
            n->axis_left = lc->splitAxis;
            if (lc->nPrimitives == 0) {
                c[0] = lc->children[0];
                c[1] = lc->children[1];
            } else {
                c[0] = lc;
            }
        }

        if (rc != NULL) {
            n->axis_right = rc->splitAxis;
            if (rc->nPrimitives == 0) {
                c[2] = rc->children[0];
                c[3] = rc->children[1];
            } else {
                c[2] = rc;
            }
        }

        align_attrib(float, 16) bboxes[2][3][4];
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                if (c[k] != NULL) {
                    bboxes[0][j][k] = c[k]->bounds.posMin().get(j);
                    bboxes[1][j][k] = c[k]->bounds.posMax().get(j);
                }
            }
        }

        for (int m = 0; m < 2; m++) {
            for (int j = 0; j < 3; j++) {
                n->bboxes[m][j] = _mm_load_ps(bboxes[m][j]);
            }
        }

        for (int i = 0; i < 4; i++) {
            if (c[i] == NULL) {
                n->children[i].leaf.flag = 1;
                n->children[i].leaf.nPrimitives = 0;
                n->children[i].leaf.index = 0;
            } else {
                if (c[i]->nPrimitives == 0) {
                    n->children[i].node.flag = 0;
                    n->children[i].node.index = _simdNodes.size();
                    collapse2QBVH(c[i]);
                } else {
                    n->children[i].leaf.flag = 1;
                    n->children[i].leaf.nPrimitives = c[i]->nPrimitives;
                    n->children[i].leaf.index = c[i]->simdTrisIdx;
                }
            }
        }
        return;
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

        float dirx = ray.direction().x();
        float diry = ray.direction().y();
        float dirz = ray.direction().z();

        align_attrib(float, 16) dirxs[4] = { dirx, dirx, dirx, dirx };
        align_attrib(float, 16) dirys[4] = { diry, diry, diry, diry };
        align_attrib(float, 16) dirzs[4] = { dirz, dirz, dirz, dirz };

        __m128 dir_x = _mm_load_ps(dirxs);
        __m128 dir_y = _mm_load_ps(dirys);
        __m128 dir_z = _mm_load_ps(dirzs);

        simdOrig[0] = _mm_load_ps(orgxs);
        simdOrig[1] = _mm_load_ps(orgys);
        simdOrig[2] = _mm_load_ps(orgzs);

        simdIdir[0] = _mm_load_ps(idirxs);
        simdIdir[1] = _mm_load_ps(idirys);
        simdIdir[2] = _mm_load_ps(idirzs);

        sgn[0] = idirx >= 0.0f ? 0 : 1;
        sgn[1] = idiry >= 0.0f ? 0 : 1;
        sgn[2] = idirz >= 0.0f ? 0 : 1;
        
        Children nodeStack[40];
        int todoNode = 0;

        nodeStack[0].raw = 0;

        bool hit = false;
        int tid = -1;

        int cnt = 0;

        while (todoNode >= 0) {
            Children item = nodeStack[todoNode--];

            if (item.node.flag == 0) {
                // This is fork node
                const SIMDBVHNode& node = *(_simdNodes[item.node.index]);
                const float hdist = (float)hitpoint->distance();
                align_attrib(float, 16) now_distance_f[4] = { hdist, hdist, hdist, hdist };
                __m128 now_distance = _mm_load_ps(now_distance_f);
                const int HitMask = test_AABB(node.bboxes, simdOrig, simdIdir, sgn, simdZero, now_distance);

                if (HitMask) {
                    const int nodeIdx = (sgn[node.axis_top] << 2) | (sgn[node.axis_left] << 1) | (sgn[node.axis_right]);
                    int bboxOrder = orderTable[HitMask * 8 + nodeIdx];

                    for (int i = 0; i < 4; i++) {
                        if (bboxOrder & 0x04) break;
                        nodeStack[++todoNode] = node.children[bboxOrder & 0x03];
                        bboxOrder >>= 4;
                    }
                }
            } else {
                // This is leaf node, SIMD-based intersection test
                align_attrib(float, 16) t_f[4];
                int nohitmask;
                SIMDTrianglePack* s = _simdTris[item.leaf.index];

                float eps = 1.0e-12f;
                align_attrib(float, 16) t0_f[4] = { 0.0f - eps, 0.0f - eps, 0.0f - eps, 0.0f - eps };
                align_attrib(float, 16) t1_f[4] = { 1.0f + eps, 1.0f + eps, 1.0f + eps, 1.0f + eps };

                __m128 t0 = _mm_load_ps(t0_f);
                __m128 t1 = _mm_load_ps(t1_f);

                __m128 e1_x = _mm_sub_ps(s->x[1], s->x[0]);
                __m128 e1_y = _mm_sub_ps(s->y[1], s->y[0]);
                __m128 e1_z = _mm_sub_ps(s->z[1], s->z[0]);

                __m128 e2_x = _mm_sub_ps(s->x[2], s->x[0]);
                __m128 e2_y = _mm_sub_ps(s->y[2], s->y[0]);
                __m128 e2_z = _mm_sub_ps(s->z[2], s->z[0]);

                __m128 s1_x = _mm_sub_ps(_mm_mul_ps(dir_y, e2_z), _mm_mul_ps(dir_z, e2_y));
                __m128 s1_y = _mm_sub_ps(_mm_mul_ps(dir_z, e2_x), _mm_mul_ps(dir_x, e2_z));
                __m128 s1_z = _mm_sub_ps(_mm_mul_ps(dir_x, e2_y), _mm_mul_ps(dir_y, e2_x));

                __m128 divisor = _mm_add_ps(_mm_add_ps(_mm_mul_ps(s1_x, e1_x), _mm_mul_ps(s1_y, e1_y)), _mm_mul_ps(s1_z, e1_z));
                __m128 no_hit  = _mm_cmpeq_ps(divisor, simdZero);

                __m128 invDivisor = _mm_rcp_ps(divisor);

                __m128 d_x = _mm_sub_ps(simdOrig[0], s->x[0]);
                __m128 d_y = _mm_sub_ps(simdOrig[1], s->y[0]);
                __m128 d_z = _mm_sub_ps(simdOrig[2], s->z[0]);

                __m128 b1 = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(d_x, s1_x), _mm_mul_ps(d_y, s1_y)), _mm_mul_ps(d_z, s1_z)), invDivisor);

                no_hit = _mm_or_ps(no_hit, _mm_or_ps(_mm_cmplt_ps(b1, t0), _mm_cmpgt_ps(b1, t1)));

                __m128 s2_x = _mm_sub_ps(_mm_mul_ps(d_y, e1_z), _mm_mul_ps(d_z, e1_y));
                __m128 s2_y = _mm_sub_ps(_mm_mul_ps(d_z, e1_x), _mm_mul_ps(d_x, e1_z));
                __m128 s2_z = _mm_sub_ps(_mm_mul_ps(d_x, e1_y), _mm_mul_ps(d_y, e1_x));

                __m128 b2 = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(dir_x, s2_x), _mm_mul_ps(dir_y, s2_y)), _mm_mul_ps(dir_z, s2_z)), invDivisor);

                no_hit = _mm_or_ps(no_hit, _mm_or_ps(_mm_cmplt_ps(b2, t0), _mm_cmpgt_ps(_mm_add_ps(b1, b2), t1)));

                __m128 t = _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(e2_x, s2_x), _mm_mul_ps(e2_y, s2_y)), _mm_mul_ps(e2_z, s2_z)), invDivisor);

                no_hit = _mm_or_ps(no_hit, _mm_cmplt_ps(t, simdEps));

                nohitmask = _mm_movemask_ps(no_hit);
                _mm_store_ps(t_f, t);

                for (int i = 0; i < 4; i++) {
                    if ((nohitmask & (1 << i)) == 0 && hitpoint->distance() > t_f[i]) {
                        tid = _ordered[s->idx[i]];
                        hit = _triangles[tid].intersect(ray, hitpoint);
                    }
                }
            }
        }

        if (!hit) tid = -1;
        return tid;
    }
}