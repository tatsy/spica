#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

TEST(RayTest, DefaultInstanceTest) {
    Ray ray;
    EXPECT_EQ_VEC(Vector3d{}, ray.org());
    EXPECT_EQ_VEC(Vector3d{}, ray.dir());
    EXPECT_EQ_VEC(Vector3d(INFTY, INFTY, INFTY), ray.invdir());
}

TEST(RayTest, InstanceTest) {
    Point3d orig(1.0, 2.0, 3.0);
    Vector3d dir(1.0, 0.0, 0.0);
    Ray ray(orig, dir);
    
    EXPECT_EQ_VEC(orig, ray.org());
    EXPECT_EQ_VEC(dir, ray.dir());
    EXPECT_EQ_VEC(Vector3d(1.0, INFTY, INFTY), ray.invdir());

    ASSERT_DEATH(Ray(orig, Vector3d(0.0, 0.0, 0.0)), "");
}

TEST(RayTest, CopyTest) {
    Point3d orig(1.0, 2.0, 3.0);
    Vector3d dir(1.0, 0.0, 0.0);
    Ray ray(orig, dir);

    Ray cp = ray;
    EXPECT_EQ_VEC(orig, cp.org());
    EXPECT_EQ_VEC(dir, cp.dir());
    EXPECT_EQ_VEC(Vector3d(1.0, INFTY, INFTY), cp.invdir());
}
