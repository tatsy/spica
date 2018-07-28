#define SPICA_API_EXPORT
#include "bvh.h"

#include <stack>
#include <functional>
#include <algorithm>

#include "core/bounds3d.h"
#include "core/interaction.h"

namespace spica {

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

int test_AABB(const __m128 bboxes[2][3], const __m128 org[3], const __m128 idir[3], const int sign[3], __m128 tmin, __m128 tmax) noexcept {
    tmin = _mm_max_ps(tmin, _mm_mul_ps(_mm_sub_ps(bboxes[sign[0]][0], org[0]), idir[0]));
    tmax = _mm_min_ps(tmax, _mm_mul_ps(_mm_sub_ps(bboxes[1 - sign[0]][0], org[0]), idir[0]));
    tmin = _mm_max_ps(tmin, _mm_mul_ps(_mm_sub_ps(bboxes[sign[1]][1], org[1]), idir[1]));
    tmax = _mm_min_ps(tmax, _mm_mul_ps(_mm_sub_ps(bboxes[1 - sign[1]][1], org[1]), idir[1]));
    tmin = _mm_max_ps(tmin, _mm_mul_ps(_mm_sub_ps(bboxes[sign[2]][2], org[2]), idir[2]));
    tmax = _mm_min_ps(tmax, _mm_mul_ps(_mm_sub_ps(bboxes[1 - sign[2]][2], org[2]), idir[2]));
    return _mm_movemask_ps(_mm_cmpge_ps(tmax, tmin));
}

struct BVHAccel::BucketInfo {
    int count;
    Bounds3d bounds;
    BucketInfo()
        : count(0)
        , bounds() {
    }
};

struct BVHAccel::ComparePoint {
    int dim;
    explicit ComparePoint(int d) : dim(d) {}
    bool operator()(const BVHPrimitiveInfo& a,
        const BVHPrimitiveInfo& b) const {
        return a.centroid[dim] < b.centroid[dim];
    }
};

struct BVHAccel::CompareToBucket {
    int splitBucket, nBuckets, dim;
    const Bounds3d& centroidBounds;

    CompareToBucket(int split, int num, int d, const Bounds3d& b)
        : splitBucket(split)
        , nBuckets(num)
        , dim(d)
        , centroidBounds(b) {
    }

    bool operator()(const BVHPrimitiveInfo& p) const {
        const double cmin = centroidBounds.posMin()[dim];
        const double cmax = centroidBounds.posMax()[dim];
        const double inv = (1.0) / (std::abs(cmax - cmin) + EPS);
        const double diff = std::abs(p.centroid[dim] - cmin);
        int b = static_cast<int>(nBuckets * diff * inv);
        if (b >= nBuckets) {
            b = nBuckets - 1;
        }
        return b <= splitBucket;
    }
};

union BVHAccel::Children {
    struct Node {
        unsigned isLeaf : 1;
        signed index : 31;
    } node;
    
    unsigned int raw;
};

struct BVHAccel::SIMDBVHNode {
    __m128 bboxes[2][3];
    Children children[4];
    int axis_top;
    int axis_left;
    int axis_right;
    int reserved;
};

#ifdef SPICA_WITH_SSE
const bool BVHAccel::useSSE_ = true;
#else
const bool BVHAccel::useSSE_ = false;
#endif

BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive>>& prims)
    : Accelerator{ prims }
    , root_{ nullptr }
    , simdNodes_{ } {
    // Construct standard BVH
    construct();
    if (useSSE_) {
        // Construct QBVH
        MsgInfo("BVH: SIMD accleration enabled!");
        collapse2QBVH(root_);
    }
}

BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive>> &prims,
                   RenderParams &params)
    : BVHAccel{ prims } {
}

BVHAccel::~BVHAccel() {
    release();
}

Bounds3d BVHAccel::worldBound() const {
    return nodes_.empty() ? Bounds3d() : nodes_[0]->bounds;
}

void BVHAccel::construct() {
    if (primitives_.empty()) return;
        
    std::vector<BVHPrimitiveInfo> primitiveInfo(primitives_.size());
    for (int i = 0; i < (int)primitives_.size(); i++) {
        primitiveInfo[i] = { i, primitives_[i]->worldBound() };
    }

    root_ = constructRec(primitiveInfo, 0, primitives_.size());
}

BVHNode* BVHAccel::constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                        int start, int end) {
    if (start == end) return nullptr;

    BVHNode* node = new BVHNode();
    nodes_.emplace_back(node);

    Bounds3d bounds;
    for (int i = start; i < end; i++) {
        bounds = Bounds3d::merge(bounds, buildData[i].bounds);
    }

    int nprims = end - start;
    if (nprims == 1) {
        // Leaf node
        node->initLeaf(bounds, buildData[start].primIdx);
    } else {
        // Fork node
        Bounds3d centroidBounds;
        for (int i = start; i < end; i++) {
            centroidBounds.merge(buildData[i].centroid);
        }

        int splitAxis = centroidBounds.maximumExtent();
        int mid = (start + end) / 2;
        if (nprims <= 8) {
            std::nth_element(buildData.begin() + start,
                                buildData.begin() + mid,
                                buildData.begin() + end,
                                ComparePoint(splitAxis));
        } else {
            // Seperate with SAH (surface area heuristics)
            const int nBuckets = 16;
            BucketInfo buckets[nBuckets];

            const double cmin = centroidBounds.posMin()[splitAxis];
            const double cmax = centroidBounds.posMax()[splitAxis];
            const double idenom = 1.0 / (std::abs(cmax - cmin) + EPS);
            for (int i = start; i < end; i++) {
                const double numer = buildData[i].centroid[splitAxis] -
                                        centroidBounds.posMin()[splitAxis];
                int b = static_cast<int>(nBuckets * std::abs(numer) * idenom);
                if (b == nBuckets) {
                    b = nBuckets - 1;
                }

                buckets[b].count++;
                buckets[b].bounds = Bounds3d::merge(buckets[b].bounds, buildData[i].bounds);
            }

            double bucketCost[nBuckets - 1] = {0};
            for (int i = 0; i < nBuckets - 1; i++) {
                Bounds3d b0, b1;
                int cnt0 = 0, cnt1 = 0;
                for (int j = 0; j <= i; j++) {
                    b0 = Bounds3d::merge(b0, buckets[j].bounds);
                    cnt0 += buckets[j].count;
                }
                for (int j = i + 1; j < nBuckets; j++) {
                    b1 = Bounds3d::merge(b1, buckets[j].bounds);
                    cnt1 += buckets[j].count;
                }
                bucketCost[i] += 0.125 + (cnt0 * b0.area() + cnt1 * b1.area()) / bounds.area();
            }

            double minCost = bucketCost[0];
            int minCostSplit = 0;
            for (int i = 1; i < nBuckets - 1; i++) {
                if (minCost > bucketCost[i]) {
                    minCost = bucketCost[i];
                    minCostSplit = i;
                }
            }

            if (minCost < nprims) {
                auto it = std::partition(buildData.begin() + start,
                                         buildData.begin() + end,
                                         CompareToBucket(minCostSplit, nBuckets, splitAxis, centroidBounds));
                mid = it - buildData.begin();
            }
        }

        BVHNode* left  = constructRec(buildData, start, mid);
        BVHNode* right = constructRec(buildData, mid, end);
        node->initFork(bounds, left, right, splitAxis);
    }
    return std::move(node);
}

void BVHAccel::release() {
    for (auto &node : simdNodes_) {
        align_free(node);
    }

    root_ = nullptr;
    simdNodes_.clear();
}

void BVHAccel::collapse2QBVH(BVHNode* node) {
    BVHNode *lc = node->left;
    BVHNode *rc = node->right;
    
    SIMDBVHNode* n = (SIMDBVHNode *)align_alloc(sizeof(SIMDBVHNode), 16);
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

bool BVHAccel::intersect(Ray& ray, SurfaceInteraction* isect) const {
    if (useSSE_) {
        return intersectQBVH(ray, isect);
    } else {
        return intersectBVH(ray, isect);
    }
}

bool BVHAccel::intersect(Ray &ray) const {
    if (useSSE_) {
        return intersectQBVH(ray);
    } else {
        return intersectBVH(ray);
    }
}

bool BVHAccel::intersectBVH(Ray &ray, SurfaceInteraction *isect) const{
    std::stack<BVHNode*> nodeStack;
    nodeStack.push(root_);

    bool hit = false;
    while (!nodeStack.empty()) {
        BVHNode* node = nodeStack.top();
        nodeStack.pop();

        if (node->isLeaf()) {
            // Leaf
            const auto& prim = primitives_[node->primIdx];
            SurfaceInteraction temp;
            if (prim->intersect(ray, &temp)) {
                *isect = temp;
                isect->setPrimitive(prim.get());
                hit = true;
            }
        } else {
            // Fork
            double tmin = INFTY, tmax = INFTY;
            if (node->bounds.intersect(ray, &tmin, &tmax)) {
                if (node->left ) nodeStack.push(node->left);
                if (node->right) nodeStack.push(node->right);
            }
        }
    }
    return hit;
}

bool BVHAccel::intersectBVH(Ray& ray) const {
    std::stack<BVHNode*> nodeStack;
    nodeStack.push(root_);

    while (!nodeStack.empty()) {
        BVHNode* node = nodeStack.top();
        nodeStack.pop();

        if (node->isLeaf()) {
            // Leaf
            const auto& prim = primitives_[node->primIdx];
            if (prim->intersect(ray)) {
                return true;
            }
        } else {
            // Fork
            double tmin = INFTY, tmax = INFTY;
            if (node->bounds.intersect(ray, &tmin, &tmax)) {
                if (node->left ) nodeStack.push(node->left);
                if (node->right) nodeStack.push(node->right);
            }
        }
    }
    return false;    
}

bool BVHAccel::intersectQBVH(Ray& ray, SurfaceInteraction* isect) const {
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

bool BVHAccel::intersectQBVH(Ray& ray) const {
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

std::vector<Triangle> BVHAccel::triangulate() const {
    std::vector<Triangle> tris;
    for (const auto& p : primitives_) {
        auto ts = p->triangulate();
        tris.insert(tris.end(), ts.begin(), ts.end());
    }
    return std::move(tris);
}

}  // namespace spica
