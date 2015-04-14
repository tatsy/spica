#include "gtest/gtest.h"

#include "../include/rainy.h"
using namespace rainy;

TEST(InstanceTest, ValidCall) {
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}