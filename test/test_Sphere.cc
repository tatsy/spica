#include "gtest/gtest.h"

#include "../include/rainy.h"
using namespace rainy;

TEST(SphereTest, InstanceTest) {
    Sphere sp(2.0, Vector3(0.0, 0.0, 0.0), Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE);

    EXPECT_EQ(0.0, sp.center().x());
    EXPECT_EQ(0.0, sp.center().y());
    EXPECT_EQ(0.0, sp.center().z());

    EXPECT_EQ(0.0, sp.emission().x());
    EXPECT_EQ(0.0, sp.emission().y());
    EXPECT_EQ(0.0, sp.emission().z());

    EXPECT_EQ(0.75, sp.color().x());
    EXPECT_EQ(0.75, sp.color().y());
    EXPECT_EQ(0.75, sp.color().z());

    EXPECT_EQ(REFLECTION_DIFFUSE, sp.reftype());

    HitPoint hitpoint;
    EXPECT_TRUE(sp.intersect(Ray(Vector3(10.0, 0.0, 0.0), Vector3(-1.0, 0.0, 0.0)), hitpoint));
    EXPECT_EQ(2.0, hitpoint.position().x());
    EXPECT_EQ(0.0, hitpoint.position().y());
    EXPECT_EQ(0.0, hitpoint.position().z());

    EXPECT_EQ(1.0, hitpoint.normal().x());
    EXPECT_EQ(0.0, hitpoint.normal().y());
    EXPECT_EQ(0.0, hitpoint.normal().z());

    EXPECT_FALSE(sp.intersect(Ray(Vector3(10.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)), hitpoint));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
