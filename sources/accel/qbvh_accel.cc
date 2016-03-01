#define SPICA_API_EXPORT
#include "qbvh_accel.h"

#include <stack>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "../core/common.h"

namespace spica {

namespace {
    
const float inff = (float)INFTY;
const float epsf = (float)EPS;
alignas(16) float infs[4]  = { inff, inff, inff, inff };
alignas(16) float zeros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
alignas(16) float epss[4]  = { epsf, epsf, epsf, epsf };
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
    Bounds3d bounds;
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

struct QBVHAccel::BVHPrimitiveInfo {
    int primitiveNumber;
    Point3d centroid;
    Bounds3d bounds;

    BVHPrimitiveInfo(int pn, const Bounds3d& b)
        : primitiveNumber(pn)
        , bounds(b) {
        centroid = (b.posMin() + b.posMax()) * 0.5;
    }
};

struct QBVHAccel::SIMDTrianglePack {
    __m128 x[3];
    __m128 y[3];
    __m128 z[3];
    int idx[4];
};

struct QBVHAccel::BVHBuildNode {
    Bounds3d bounds;
    BVHBuildNode* children[2];
    int splitAxis, firstPrimOffset, nPrimitives;
    int simdTrisIdx;

    BVHBuildNode()
        : bounds()
        , children()
        , splitAxis(-1)
        , firstPrimOffset(-1)
        , nPrimitives(0)
        , simdTrisIdx(-1) {
        children[0] = children[1] = nullptr;
    }

    void InitLeaf(int first, int n, const Bounds3d& b, const int asimdTrisIdx) {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
        simdTrisIdx = asimdTrisIdx;
        splitAxis = 0;
    }

    void InitInterior(int axis, BVHBuildNode* c0, BVHBuildNode* c1) {
        children[0] = c0;
        children[1] = c1;
        bounds = Bounds3d::merge(c0->bounds, c1->bounds);
        splitAxis = axis;
        firstPrimOffset = -1;
        nPrimitives = 0;
    }
};

struct QBVHAccel::ComparePoint {
    int dim;
    explicit ComparePoint(int d) : dim(d) {}
    bool operator()(const BVHPrimitiveInfo &a, const BVHPrimitiveInfo &b) const {
        return a.centroid[dim] < b.centroid[dim];
    }
};

struct QBVHAccel::CompareToBucket {
    int splitBucket, nBuckets, dim;
    const Bounds3d& centroidBounds;

    CompareToBucket(int split, int num, int d, const Bounds3d& b)
        : centroidBounds(b)
        , splitBucket(split)
        , nBuckets(num)
        , dim(d) {
    }

    bool operator()(const BVHPrimitiveInfo& p) const {
        int b = (int)(nBuckets * ((p.centroid[dim] - centroidBounds.posMin()[dim]) / (centroidBounds.posMax()[dim] - centroidBounds.posMin()[dim])));
        if (b == nBuckets) {
            b = nBuckets - 1;
        }
        return b <= splitBucket;
    }
};

union QBVHAccel::Children {
    struct Node {
        unsigned flag : 1;
        unsigned index : 31;
    } node;

    struct Leaf {
        unsigned flag : 1;
        unsigned nPrimitives : 3;
        unsigned index : 28;
    } leaf;

    unsigned int raw;
};

struct QBVHAccel::SIMDBVHNode {
    __m128 bboxes[2][3];
    Children children[4];
    int axis_top;
    int axis_left;
    int axis_right;
    int reserved;
};

QBVHAccel::QBVHAccel()
    : AccelInterface{ AccelType::QBVH }
    , root_{ nullptr }
    , triangles_{}
    , ordered_{}
    , simdNodes_{}
    , simdTris_{} {
}

QBVHAccel::~QBVHAccel() {
    release();
}

void QBVHAccel::release() {
    for (int i = 0; i < simdNodes_.size(); i++) {
        align_free(simdNodes_[i]);
    }

    for (int i = 0; i < simdTris_.size(); i++) {
        align_free(simdTris_[i]);
    }

    root_ = nullptr;
    ordered_.clear();
    triangles_.clear();
    simdNodes_.clear();
    simdTris_.clear();
}

void QBVHAccel::construct() {
    // Destruct previos tree
    release();

    /*
    triangles_ = triangles;

    std::vector<BVHPrimitiveInfo> buildData;
    for (int i = 0; i < triangles.size(); i++) {
        Bounds3d b = triangles[i].worldBound();
        buildData.emplace_back(i, b);
    }

    int totalNodes = 0;
    ordered_.reserve(triangles.size());

    root_ = constructRec(buildData, 0, triangles.size(), &totalNodes, ordered_);

    collapse2QBVH(root_);
     */
}
    
QBVHAccel::BVHBuildNode*
QBVHAccel::constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                        int start, int end, int* totalNodes, 
                        std::vector<int>& orderedPrims) {
    (*totalNodes)++;
    BVHBuildNode* node = new BVHBuildNode();

    Bounds3d bbox;
    for (int i = start; i < end; i++) {
        bbox.merge(buildData[i].bounds);
    }

    int nPrimitives = end - start;
    if (nPrimitives <= 4) {
        // This is leaf node
        int firstPrimOffset = orderedPrims.size();
            
        SIMDTrianglePack* simdt = reinterpret_cast<SIMDTrianglePack*>(align_alloc(sizeof(SIMDTrianglePack), 16));
        Assertion(simdt != nullptr, "allocation failed !!");

        alignas(16) float x[4 * 3] = {0};
        alignas(16) float y[4 * 3] = {0};
        alignas(16) float z[4 * 3] = {0};

        int cnt = 0;
        for (int i = start; i < end; i++, cnt++) {
            const int idx = buildData[i].primitiveNumber;
            orderedPrims.push_back(idx);

            int t = cnt % 4;

            simdt->idx[t] = firstPrimOffset + cnt;
            for (int k = 0; k < 3; k++) {
                x[4 * k + t] = triangles_[idx][k].x();
                y[4 * k + t] = triangles_[idx][k].y();
                z[4 * k + t] = triangles_[idx][k].z();
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

        simdTris_.push_back(simdt);
        node->InitLeaf(firstPrimOffset, nPrimitives, bbox, simdTris_.size() - 1);
    } else {
        // This is fork node
        Bounds3d centroidBounds;
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
                int b = (int)(nBuckets * (buildData[i].centroid[dim] - centroidBounds.posMin()[dim]) / 
                                (centroidBounds.posMax()[dim] - centroidBounds.posMin()[dim]));
                if (b == nBuckets) {
                    b = nBuckets - 1;
                }
                buckets[b].count++;
                buckets[b].bounds.merge(buildData[i].bounds);
            }

            float cost[nBuckets - 1] = {0};
            for (int i = 0; i < nBuckets - 1; i++) {
                Bounds3d b0, b1;
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
    n = reinterpret_cast<SIMDBVHNode*>(align_alloc(sizeof(SIMDBVHNode), 16));
    Assertion(n != nullptr, "allocation failed !!");

    simdNodes_.push_back(n);
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

    alignas(16) float bboxes[2][3][4];
    for (int j = 0; j < 3; j++) {
        for (int k = 0; k < 4; k++) {
            if (c[k] != NULL) {
                bboxes[0][j][k] = c[k]->bounds.posMin()[j];
                bboxes[1][j][k] = c[k]->bounds.posMax()[j];
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
                n->children[i].node.index = simdNodes_.size();
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

bool QBVHAccel::intersect(Ray& ray, SurfaceInteraction* isect) const {
    // ray for SIMD arthimetic
    __m128 simdOrig[3];  // origin
    __m128 simdIdir[3];  // inverse direction
    int sgn[3];          // signs of ray direction (pos -> 0, neg -> 1)
        
    float orgx = static_cast<float>(ray.org().x());
    float orgy = static_cast<float>(ray.org().y());
    float orgz = static_cast<float>(ray.org().z());

    alignas(16) float orgxs[4] = { orgx, orgx, orgx, orgx };
    alignas(16) float orgys[4] = { orgy, orgy, orgy, orgy };
    alignas(16) float orgzs[4] = { orgz, orgz, orgz, orgz };

    float idirx = ray.dir().x() == 0.0 ? (float)1.0e20 : (float)(1.0 / ray.dir().x());
    float idiry = ray.dir().y() == 0.0 ? (float)1.0e20 : (float)(1.0 / ray.dir().y());
    float idirz = ray.dir().z() == 0.0 ? (float)1.0e20 : (float)(1.0 / ray.dir().z());

    alignas(16) float idirxs[4] = { idirx, idirx, idirx, idirx };
    alignas(16) float idirys[4] = { idiry, idiry, idiry, idiry };
    alignas(16) float idirzs[4] = { idirz, idirz, idirz, idirz };

    float dirx = ray.dir().x();
    float diry = ray.dir().y();
    float dirz = ray.dir().z();

    alignas(16) float dirxs[4] = { dirx, dirx, dirx, dirx };
    alignas(16) float dirys[4] = { diry, diry, diry, diry };
    alignas(16) float dirzs[4] = { dirz, dirz, dirz, dirz };

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

    int tid = -1;
    double tHit = ray.maxDist();
    while (todoNode >= 0) {
        Children item = nodeStack[todoNode--];

        if (item.node.flag == 0) {
            // This is fork node
            const SIMDBVHNode& node = *(simdNodes_[item.node.index]);
            const float tHitf = static_cast<float>(tHit);
            alignas(16) float now_distance_f[4] = { tHitf, tHitf, tHitf, tHitf };
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
            alignas(16) float t_f[4];
            int nohitmask;
            SIMDTrianglePack* s = simdTris_[item.leaf.index];

            float eps = 1.0e-12f;
            alignas(16) float t0_f[4] = { 0.0f - eps, 0.0f - eps, 0.0f - eps, 0.0f - eps };
            alignas(16) float t1_f[4] = { 1.0f + eps, 1.0f + eps, 1.0f + eps, 1.0f + eps };

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
                if ((nohitmask & (1 << i)) == 0 && tHit > t_f[i]) {
                    triangles_[tid].intersect(ray, &tHit, isect);
                }
            }
        }
    }
    return tHit < ray.maxDist();
}

}  // namespace spica
