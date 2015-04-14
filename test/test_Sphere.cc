#include "gtest/gtest.h"

#include "../include/rainy.h"
using namespace rainy;

TEST(SphereTest, InstanceTest) {
    Sphere sp(1.0, Vector3(0.0, 0.0, 0.0), Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE);

    EXPECT_EQ(0.0, sp.center().x());
    EXPECT_EQ(0.0, sp.center().y());
    EXPECT_EQ(0.0, sp.center().z());

    HitPoint hitpoint;
    sp.intersect(Ray(Vector3(10.0, 0.0, 0.0), Vector3(-1.0, 0.0, 0.0)), hitpoint);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
