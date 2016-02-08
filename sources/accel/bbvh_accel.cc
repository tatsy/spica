#define SPICA_API_EXPORT
#include "bbvh_accel.h"

#include <stack>
#include <functional>
#include <algorithm>

#include "../shape/bbox.h"

namespace spica {

    struct BBVHAccel::BVHPrimitiveInfo {
        int primIdx;
        Vector3D centroid;
        BBox bounds;

        BVHPrimitiveInfo(int pid, const BBox& b)
            : primIdx(pid)
            , centroid()
            , bounds(b) {
            centroid = (b.posMax() + b.posMin()) * 0.5;
        }
    };

    struct BBVHAccel::BucketInfo {
        int count;
        BBox bounds;
        BucketInfo()
            : count(0)
            , bounds() {
        }
    };

    struct BBVHAccel::BBvhNode {
        BBox bounds;
        BBvhNode* left;
        BBvhNode* right;
        int splitAxis;
        int triIdx;

        void initLeaf(const BBox& b, int tid) {
            this->bounds = bounds;
            this->splitAxis = -1;
            this->triIdx = tid;
        }

        void initFork(const BBox& b, BBvhNode* l, BBvhNode* r, int axis) {
            this->bounds = b;
            this->left  = l;
            this->right = r;
            this->splitAxis = axis;
            this->triIdx = -1;
        }

        bool isLeaf() const {
            return triIdx >= 0;
        }
    };

    struct BBVHAccel::ComparePoint {
        int dim;
        ComparePoint(int d) : dim(d) {}
        bool operator()(const BVHPrimitiveInfo& a,
            const BVHPrimitiveInfo& b) const {
            return a.centroid.get(dim) < b.centroid.get(dim);
        }
    };

    struct BBVHAccel::CompareToBucket {
        int splitBucket, nBuckets, dim;
        const BBox& centroidBounds;

        CompareToBucket(int split, int num, int d, const BBox& b)
            : splitBucket(split)
            , nBuckets(num)
            , dim(d)
            , centroidBounds(b) {
        }

        bool operator()(const BVHPrimitiveInfo& p) const {
            const double cmin = centroidBounds.posMin().get(dim);
            const double cmax = centroidBounds.posMax().get(dim);
            const double inv = (1.0) / (std::abs(cmax - cmin) + EPS);
            const double diff = std::abs(p.centroid.get(dim) - cmin);
            int b = static_cast<int>(nBuckets * diff * inv);
            if (b >= nBuckets) {
                b = nBuckets - 1;
            }
            return b <= splitBucket;
        }
    };

    BBVHAccel::BBVHAccel()
        : IAccel{AccelType::BBVH}
        , _root(nullptr)
        , _tris()
        , _nodes() {
    }

    BBVHAccel::~BBVHAccel() {
    }

    void BBVHAccel::release() {
        for (int i = 0; i < _nodes.size(); i++) {
            _nodes[i].reset();
        }

        _root = nullptr;
        _tris.clear();
        _tris.shrink_to_fit();

        _nodes.clear();
        _nodes.shrink_to_fit();
    }

    void BBVHAccel::construct(const std::vector<Triangle>& triangles) {
        // Deallocate previous tree
        release();

        _tris.assign(triangles.begin(), triangles.end());

        std::vector<BVHPrimitiveInfo> buildData;
        for (int i = 0; i < _tris.size(); i++) {
            const BBox b = BBox::fromTriangle(_tris[i]);
            buildData.emplace_back(i, b);
        }

        _root = constructRec(buildData, 0, _tris.size());
    }

    BBVHAccel::BBvhNode*
    BBVHAccel::constructRec(std::vector<BVHPrimitiveInfo>& buildData,
                            int start, int end) {
        if (start == end) return nullptr;

        BBvhNode* node = new BBvhNode();
        _nodes.emplace_back(node);

        BBox bounds;
        for (int i = start; i < end; i++) {
            bounds.merge(buildData[i].bounds);
        }

        int nprims = end - start;
        if (nprims == 1) {
            // Leaf node
            node->initLeaf(bounds, buildData[start].primIdx);
        } else {
            // Fork node
            BBox centroidBounds;
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

                const double cmin = centroidBounds.posMin().get(splitAxis);
                const double cmax = centroidBounds.posMax().get(splitAxis);
                const double idenom = 1.0 / (std::abs(cmax - cmin) + EPS);
                for (int i = start; i < end; i++) {
                    const double numer = buildData[i].centroid.get(splitAxis) -
                                         centroidBounds.posMin().get(splitAxis);
                    int b = static_cast<int>(nBuckets * std::abs(numer) * idenom);
                    if (b == nBuckets) {
                        b = nBuckets - 1;
                    }

                    buckets[b].count++;
                    buckets[b].bounds.merge(buildData[i].bounds);
                }

                double bucketCost[nBuckets - 1] = {0};
                for (int i = 0; i < nBuckets - 1; i++) {
                    BBox b0, b1;
                    int cnt0 = 0, cnt1 = 0;
                    for (int j = 0; j <= i; j++) {
                        b0.merge(buckets[j].bounds);
                        cnt0 += buckets[j].count;
                    }
                    for (int j = i + 1; j < nBuckets; j++) {
                        b1.merge(buckets[j].bounds);
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

            BBvhNode* left  = constructRec(buildData, start, mid);
            BBvhNode* right = constructRec(buildData, mid, end);
            node->initFork(bounds, left, right, splitAxis);
        }
        return std::move(node);
    }

    int BBVHAccel::intersect(const Ray& ray, Hitpoint* hpoint) const {
        std::stack<BBvhNode*> nodeStack;
        nodeStack.push(_root);

        int hitID = -1;
        while (!nodeStack.empty()) {
            BBvhNode* node = nodeStack.top();
            nodeStack.pop();

            if (node->isLeaf()) {
                // Leaf
                const Triangle& tri = _tris[node->triIdx];
                Hitpoint temp;
                if (tri.intersect(ray, &temp)) {
                    if (temp.distance() < hpoint->distance()) {
                        (*hpoint) = temp;
                        hitID = node->triIdx;
                    }
                }
            } else {
                // Fork
                double tmin = INFTY, tmax = INFTY;
                if (node->bounds.intersect(ray, &tmin, &tmax)) {
                    if (tmin < hpoint->distance()) {
                        if (node->left ) nodeStack.push(node->left);
                        if (node->right) nodeStack.push(node->right);
                    }
                }
            }
        }
        return hitID;
    }

}  // namespace spica
