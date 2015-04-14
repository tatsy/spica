#include "gtest/gtest.h"

#include "../include/rainy.h"
using namespace rainy;

TEST(Vector3Test, InstanceTest) {
    Vector3 v;
    EXPECT_EQ(0.0, v.x());
    EXPECT_EQ(0.0, v.y());
    EXPECT_EQ(0.0, v.z());

    Vector3 u(1.0, 2.0, 3.0);
    EXPECT_EQ(1.0, u.x());
    EXPECT_EQ(2.0, u.y());
    EXPECT_EQ(3.0, u.z());

    v = Vector3(u);
    EXPECT_EQ(1.0, v.x());
    EXPECT_EQ(2.0, v.y());
    EXPECT_EQ(3.0, v.z());

    v = Vector3(2.0, 4.0, 6.0);
    EXPECT_EQ(2.0, v.x());
    EXPECT_EQ(4.0, v.y());
    EXPECT_EQ(6.0, v.z());
}

TEST(Vector3Test, AlgebraTest) {
    Vector3 w;
    Vector3 v(1.0, 2.0, 3.0);
    Vector3 u(1.0, 2.0, 3.0);

    w = u + v;
    EXPECT_EQ(2.0, w.x());
    EXPECT_EQ(4.0, w.y());
    EXPECT_EQ(6.0, w.z());

    w = u - v;
    EXPECT_EQ(0.0, w.x());
    EXPECT_EQ(0.0, w.y());
    EXPECT_EQ(0.0, w.z());

    w = u * 5.0;
    EXPECT_EQ(5.0, w.x());
    EXPECT_EQ(10.0, w.y());
    EXPECT_EQ(15.0, w.z());

    w = u / 2.0;
    EXPECT_EQ(0.5, w.x());
    EXPECT_EQ(1.0, w.y());
    EXPECT_EQ(1.5, w.z());

    ASSERT_DEATH(u /= 0.0, "");

    double dt = u.dot(v);
    EXPECT_EQ(14.0, dt);
    EXPECT_EQ(dt, v.dot(u));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}