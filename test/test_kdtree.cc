#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

#include <vector>
#include <algorithm>

struct Pair {
    double d;
    Vector3 v;
    Pair(double d_, Vector3 v_) 
        : d(d_)
        , v(v_)
    {
    }
    bool operator<(const Pair& p) const {
        return this->d < p.d;
    }
    bool operator>(const Pair& p) const {
        return this->d > p.d;
    }
};

TEST(KdTreeTest, KNNTest) {
    const int numSample = 100;
    Random rng = Random::getRNG();
    std::vector<Vector3> points;
    for (int i = 0; i < numSample; i++) {
        double x = rng.randReal() * 10.0 - 5.0;
        double y = rng.randReal() * 10.0 - 5.0;
        double z = rng.randReal() * 10.0 - 5.0;
        points.push_back(Vector3(x, y, z));
    }

    KdTree<Vector3> kdtree;
    kdtree.construct(points);

    Vector3 query;
    std::vector<Vector3> results;
    const int K = 10;
    const int eps = 0.5;
    kdtree.knnSearch(query, KnnQuery(K_NEAREST | EPSILON_BALL, 0.5, 10), &results);
    EXPECT_LE(results.size(), K);

    std::vector<Pair> expected;
    std::vector<Pair> actual;
    for (int i = 0; i< numSample; i++) {
        double dist = (points[i] - query).norm();
        expected.push_back(Pair(dist, points[i]));
    }

    for (int i = 0; i < results.size(); i++) {
        double dist = (results[i] - query).norm();
        actual.push_back(Pair(dist, results[i]));
        EXPECT_LE(dist, eps);
    }

    for (int i = 0; i < results.size(); i++) {
        EXPECT_EQ_VEC(expected[i].v, actual[i].v);
    }
}