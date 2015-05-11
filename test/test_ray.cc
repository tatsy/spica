#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

TEST(RayTest, DefaultInstanceTest) {
    Ray ray;
    EXPECT_EQ_VEC(Vector3(), ray.origin());
    EXPECT_EQ_VEC(Vector3(), ray.direction());
    EXPECT_EQ_VEC(Vector3(INFTY, INFTY, INFTY), ray.invdir());
}

TEST(RayTest, InstanceTest) {
    Vector3 orig(1.0, 2.0, 3.0);
    Vector3 dir(1.0, 0.0, 0.0);
    Ray ray(orig, dir);
    
    EXPECT_EQ_VEC(orig, ray.origin());
    EXPECT_EQ_VEC(dir, ray.direction());
    EXPECT_EQ_VEC(Vector3(1.0, INFTY, INFTY), ray.invdir());

    ASSERT_DEATH(Ray(orig, Vector3(1.0, 1.0, 1.0)), "");
}

TEST(RayTest, CopyTest) {
    Vector3 orig(1.0, 2.0, 3.0);
    Vector3 dir(1.0, 0.0, 0.0);
    Ray ray(orig, dir);

    Ray cp = ray;
    EXPECT_EQ_VEC(orig, cp.origin());
    EXPECT_EQ_VEC(dir, cp.direction());
    EXPECT_EQ_VEC(Vector3(1.0, INFTY, INFTY), cp.invdir());
}
