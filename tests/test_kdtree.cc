#include "gtest/gtest.h"
#include "spica.h"
using namespace spica;

#include <vector>
#include <algorithm>

struct Pair {
    double d;
    Vector3d v;
    Pair(double d_, const Vector3d &v_)
        : d(d_)
        , v(v_)
    {
    }
    bool operator<(const Pair &p) const {
        return this->d < p.d;
    }
    bool operator>(const Pair &p) const {
        return this->d > p.d;
    }
};

TEST(KdTreeTest, KNNTest) {
    const int numSample = 1000;
    Random rng = Random();
    std::vector<Vector3d> points;
    for (int i = 0; i < numSample; i++) {
        double x = rng.nextReal() * 10.0 - 5.0;
        double y = rng.nextReal() * 10.0 - 5.0;
        double z = rng.nextReal() * 10.0 - 5.0;
        points.emplace_back(x, y, z);
    }

    KdTree<Vector3d> kdtree;
    kdtree.construct(points);

    Vector3d query;
    std::vector<Vector3d> results;
    const int K = 10;
    const double eps = 2.0;
    kdtree.knnSearch(query, KnnQuery(K_NEAREST | EPSILON_BALL, K, eps), &results);

    std::vector<Pair> expected;
    std::vector<Pair> actual;
    int cnt = 0;
    for (const auto &p : points) {
        double dist = (p - query).norm();
        expected.emplace_back(dist, p);
        if (dist <= eps) {
            cnt++;
        }
    }

    for (const auto & r : results) {
        double dist = (r - query).norm();
        actual.emplace_back(dist, r);
        EXPECT_LE(dist, eps);
    }

    std::sort(expected.begin(), expected.end());
    std::sort(actual.begin(), actual.end());

    cnt = std::min(cnt, K);
    EXPECT_EQ(cnt, results.size());
    for (int i = 0; i < (int)results.size(); i++) {
        EXPECT_EQ(expected[i].d, actual[i].d);
        EXPECT_EQ(expected[i].v.x(), actual[i].v.x());
        EXPECT_EQ(expected[i].v.y(), actual[i].v.y());
        EXPECT_EQ(expected[i].v.z(), actual[i].v.z());
    }
}