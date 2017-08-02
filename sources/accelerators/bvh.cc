#define SPICA_API_EXPORT
#include "bvh.h"

#include <stack>
#include <functional>
#include <algorithm>

#include "core/bounds3d.h"
#include "core/interaction.h"

namespace spica {

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

BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive>>& prims)
    : Accelerator{ prims }
    , root_{ nullptr } {
    construct();
}

BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive>> &prims,
                   RenderParams &params)
    : BVHAccel{prims} {
}

BVHAccel::~BVHAccel() {
}

Bounds3d BVHAccel::worldBound() const {
    return nodes_.empty() ? Bounds3d() : nodes_[0]->bounds;
}

void BVHAccel::construct() {
    if (primitives_.empty()) return;
        
    std::vector<BVHPrimitiveInfo> primitiveInfo(primitives_.size());
    for (int i = 0; i < primitives_.size(); i++) {
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

bool BVHAccel::intersect(Ray& ray, SurfaceInteraction* isect) const {
    std::stack<BVHNode*> nodeStack;
    nodeStack.push(root_);

    bool hit = false;
    int cnt = 0;
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

bool BVHAccel::intersect(Ray& ray) const {
std::stack<BVHNode*> nodeStack;
    nodeStack.push(root_);

    bool hit = false;
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

std::vector<Triangle> BVHAccel::triangulate() const {
    std::vector<Triangle> tris;
    for (const auto& p : primitives_) {
        auto ts = p->triangulate();
        tris.insert(tris.end(), ts.begin(), ts.end());
    }
    return std::move(tris);
}

}  // namespace spica
