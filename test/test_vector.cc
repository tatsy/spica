#include "gtest/gtest.h"

#include <algorithm>

#include "../include/spica.h"
using namespace spica;

TEST(Vector3DTest, InstanceTest) {
    Vector3D v;
    EXPECT_EQ(0.0, v.x());
    EXPECT_EQ(0.0, v.y());
    EXPECT_EQ(0.0, v.z());

    Vector3D u(1.0, 2.0, 3.0);
    EXPECT_EQ(1.0, u.x());
    EXPECT_EQ(2.0, u.y());
    EXPECT_EQ(3.0, u.z());

    v = Vector3D(u);
    EXPECT_EQ(1.0, v.x());
    EXPECT_EQ(2.0, v.y());
    EXPECT_EQ(3.0, v.z());

    v = Vector3D(2.0, 4.0, 6.0);
    EXPECT_EQ(2.0, v.x());
    EXPECT_EQ(4.0, v.y());
    EXPECT_EQ(6.0, v.z());
}

TEST(Vector3DTest, AlgebraTest) {
    Vector3D w;
    Vector3D v(1.0, 2.0, 3.0);
    Vector3D u(1.0, 2.0, 3.0);

    w = u + v;
    EXPECT_EQ(2.0, w.x());
    EXPECT_EQ(4.0, w.y());
    EXPECT_EQ(6.0, w.z());

    w = u - v;
    EXPECT_EQ(0.0, w.x());
    EXPECT_EQ(0.0, w.y());
    EXPECT_EQ(0.0, w.z());

    w = 5.0 * u * 5.0;
    EXPECT_EQ(25.0, w.x());
    EXPECT_EQ(50.0, w.y());
    EXPECT_EQ(75.0, w.z());

    w = u / 2.0;
    EXPECT_EQ(0.5, w.x());
    EXPECT_EQ(1.0, w.y());
    EXPECT_EQ(1.5, w.z());

    ASSERT_DEATH(u /= 0.0, "");

    w = u.multiply(v);
    EXPECT_EQ(1.0, w.x());
    EXPECT_EQ(4.0, w.y());
    EXPECT_EQ(9.0, w.z());

    double nrm = u.norm();
    EXPECT_EQ(sqrt(14.0), nrm);

    w = u.normalized();
    EXPECT_EQ(u.x() / nrm, w.x());
    EXPECT_EQ(u.y() / nrm, w.y());
    EXPECT_EQ(u.z() / nrm, w.z());
    EXPECT_FLOAT_EQ(w.norm(), 1.0);
    EXPECT_NE(u.norm(), 1.0);

    Vector3D z = Vector3D::normalize(u);
    EXPECT_EQ(u.x() / nrm, z.x());
    EXPECT_EQ(u.y() / nrm, z.y());
    EXPECT_EQ(u.z() / nrm, z.z());
    EXPECT_FLOAT_EQ(1.0, z.norm());
    EXPECT_NE(u.norm(), 1.0);
}

TEST(Vector3DTest, NegateTest) {
    Vector3D u(1.0, 2.0, 3.0);
    Vector3D v = -u;
    EXPECT_EQ(-u.x(), v.x());
    EXPECT_EQ(-u.y(), v.y());
    EXPECT_EQ(-u.z(), v.z());
}

TEST(Vector3DTest, SqrtTest) {
    Vector3D u(1.0, 2.0, 3.0);
    Vector3D v = Vector3D::sqrt(u);
    EXPECT_EQ(sqrt(u.x()), v.x());
    EXPECT_EQ(sqrt(u.y()), v.y());
    EXPECT_EQ(sqrt(u.z()), v.z());

    u = Vector3D(-1.0, 2.0, 3.0);
    ASSERT_DEATH(Vector3D::sqrt(u), "");
}

TEST(Vector3DTest, ExpTest) {
    Vector3D u(1.0, 2.0, 3.0);
    Vector3D v = Vector3D::exp(u);
    EXPECT_EQ(exp(u.x()), v.x());
    EXPECT_EQ(exp(u.y()), v.y());
    EXPECT_EQ(exp(u.z()), v.z());
}


TEST(Vector3DTest, DotCrossTest) {
    Vector3D u(1.0, 2.0, 3.0);
    Vector3D v(4.0, 5.0, 6.0);
    double dt = u.dot(v);
    EXPECT_EQ(32.0, dt);
    EXPECT_EQ(dt, v.dot(u));

    Vector3D w = u.cross(v);
    EXPECT_EQ(-3.0, w.x());
    EXPECT_EQ(6.0, w.y());
    EXPECT_EQ(-3.0, w.z());
}

TEST(Vector3DTest, MaxMinTest) {
    static const int nTrial = 100;
    Random rng = Random();

    Vector3D minv(INFTY, INFTY, INFTY);
    double minx = INFTY;
    double miny = INFTY;
    double minz = INFTY;

    for (int i = 0; i < nTrial; i++) {
        double x = rng.nextReal();
        double y = rng.nextReal();
        double z = rng.nextReal();
        minv = Vector3D::minimum(minv, Vector3D(x, y, z));
        minx = std::min(minx, x);
        miny = std::min(miny, y);
        minz = std::min(minz, z);

        EXPECT_EQ(minx, minv.x());
        EXPECT_EQ(miny, minv.y());
        EXPECT_EQ(minz, minv.z());
    }

    Vector3D maxv(-INFTY, -INFTY, -INFTY);
    double maxx = -INFTY;
    double maxy = -INFTY;
    double maxz = -INFTY;

    for (int i = 0; i < nTrial; i++) {
        double x = rng.nextReal();
        double y = rng.nextReal();
        double z = rng.nextReal();
        maxv = Vector3D::maximum(maxv, Vector3D(x, y, z));
        maxx = std::max(maxx, x);
        maxy = std::max(maxy, y);
        maxz = std::max(maxz, z);

        EXPECT_EQ(maxx, maxv.x());
        EXPECT_EQ(maxy, maxv.y());
        EXPECT_EQ(maxz, maxv.z());
    }
}
