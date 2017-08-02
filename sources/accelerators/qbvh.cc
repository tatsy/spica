#define SPICA_API_EXPORT
#include "qbvh.h"

#include <stack>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "render/interaction.h"

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

union QBVHAccel::Children {
    struct Node { 
        unsigned isLeaf : 1;
        signed index : 31;
    } node;

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

QBVHAccel::QBVHAccel(const std::vector<std::shared_ptr<Primitive>>& prims)
    : BBVHAccel{ prims }
    , simdNodes_{} {
    collapse2QBVH(root_);
}

QBVHAccel::~QBVHAccel() {
    release();
}

void QBVHAccel::release() {
    for (int i = 0; i < simdNodes_.size(); i++) {
        align_free(simdNodes_[i]);
    }

    root_ = nullptr;
    simdNodes_.clear();
}

void QBVHAccel::collapse2QBVH(BVHNode* node) {
    BVHNode *lc = node->left;
    BVHNode *rc = node->right;

    SIMDBVHNode* n = 
        static_cast<SIMDBVHNode*>(align_alloc(sizeof(SIMDBVHNode), 16));
    Assertion(n != nullptr, "allocation failed !!");

    simdNodes_.push_back(n);
    n->axis_top = node->splitAxis;
    n->axis_left = n->axis_right = 0;

    BVHNode* c[4] = {0};
    if (lc != nullptr) {
        n->axis_left = lc->splitAxis;
        if (!lc->isLeaf()) {
            c[0] = lc->left;
            c[1] = lc->right;
        } else {
            c[0] = lc;
        }
    }

    if (rc != nullptr) {
        n->axis_right = rc->splitAxis;
        if (!rc->isLeaf()) {
            c[2] = rc->left;
            c[3] = rc->right;
        } else {
            c[2] = rc;
        }
    }

    alignas(16) float bboxes[2][3][4];
    for (int j = 0; j < 3; j++) {
        for (int k = 0; k < 4; k++) {
            if (c[k] != nullptr) {
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
        if (c[i] == nullptr) {
            n->children[i].node.isLeaf = 1;
            n->children[i].node.index = -1;
        } else {
            if (!c[i]->isLeaf()) {
                n->children[i].node.isLeaf = 0;
                n->children[i].node.index  = simdNodes_.size();
                collapse2QBVH(c[i]);
            } else {
                n->children[i].node.isLeaf = 1;
                n->children[i].node.index  = c[i]->primIdx;
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

    bool hit = false;
    while (todoNode >= 0) {
        Children item = nodeStack[todoNode--];

        if (item.node.isLeaf == 0) {
            // Fork
            const SIMDBVHNode& node = *(simdNodes_[item.node.index]);
            const float tHitf = static_cast<float>(ray.maxDist());
            alignas(16) float now_distance_f[4] = { tHitf, tHitf, tHitf, tHitf };
            __m128 now_distance = _mm_load_ps(now_distance_f);
            const int hitMask = test_AABB(node.bboxes, simdOrig, simdIdir, sgn, simdZero, now_distance);

            if (hitMask != 0) {
                const int nodeIdx = (sgn[node.axis_top] << 2) | (sgn[node.axis_left] << 1) | (sgn[node.axis_right]);
                int bboxOrder = orderTable[hitMask * 8 + nodeIdx];

                for (int i = 0; i < 4; i++) {
                    if (bboxOrder & 0x04) break;
                    nodeStack[++todoNode] = node.children[bboxOrder & 0x03];
                    bboxOrder >>= 4;
                }
            }
        } else {
            // Leaf
            if (item.node.index >= 0) {
                const auto& prim = primitives_[item.node.index];
                SurfaceInteraction temp;
                if (primitives_[item.node.index]->intersect(ray, &temp)) {
                    *isect = temp;
                    isect->setPrimitive(prim.get());
                    hit = true;
                }
            }
        }
    }
    return hit;
}

bool QBVHAccel::intersect(Ray& ray) const {
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

    while (todoNode >= 0) {
        Children item = nodeStack[todoNode--];

        if (item.node.isLeaf == 0) {
            // Fork
            const SIMDBVHNode& node = *(simdNodes_[item.node.index]);
            const float tHitf = static_cast<float>(ray.maxDist());
            alignas(16) float now_distance_f[4] = { tHitf, tHitf, tHitf, tHitf };
            __m128 now_distance = _mm_load_ps(now_distance_f);
            const int hitMask = test_AABB(node.bboxes, simdOrig, simdIdir, sgn, simdZero, now_distance);

            if (hitMask != 0) {
                const int nodeIdx = (sgn[node.axis_top] << 2) | (sgn[node.axis_left] << 1) | (sgn[node.axis_right]);
                int bboxOrder = orderTable[hitMask * 8 + nodeIdx];

                for (int i = 0; i < 4; i++) {
                    if (bboxOrder & 0x04) break;
                    nodeStack[++todoNode] = node.children[bboxOrder & 0x03];
                    bboxOrder >>= 4;
                }
            }
        } else {
            // Leaf
            if (item.node.index >= 0) {
                const auto& prim = primitives_[item.node.index];
                if (prim->intersect(ray)) {
                    return true;
                }
            }
        }
    }
    return false;
}

}  // namespace spica
