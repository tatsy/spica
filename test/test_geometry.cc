#include "gtest/gtest.h"

#include <cmath>

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

// ------------------------------
// Sphere class test
// ------------------------------

TEST(SphereTest, InstanceTest) {
    Sphere sp0;
    EXPECT_EQ(0.0, sp0.center().x());
    EXPECT_EQ(0.0, sp0.center().y());
    EXPECT_EQ(0.0, sp0.center().z());
    EXPECT_EQ(0.0, sp0.radius());

    Sphere sp(Point3d(0.0, 0.0, 0.0), 2.0);
    EXPECT_EQ(0.0, sp.center().x());
    EXPECT_EQ(0.0, sp.center().y());
    EXPECT_EQ(0.0, sp.center().z());
}

TEST(SphereTest, CopyConstructor) {
    Sphere sp(Point3d(0.0, 0.0, 0.0), 2.0);
    Sphere sp0 = sp;
    EXPECT_EQ(0.0, sp0.center().x());
    EXPECT_EQ(0.0, sp0.center().y());
    EXPECT_EQ(0.0, sp0.center().z());
}

TEST(SphereTest, IntersectionTest) {
    Sphere sp(Point3d(0.0, 0.0, 0.0), 2.0);
    SurfaceInteraction isect;
    double tHit;
    EXPECT_TRUE(sp.intersect(
        Ray(Point3d(10.0, 0.0, 0.0), Vector3d(-1.0, 0.0, 0.0)), &tHit, &isect));
    EXPECT_EQ(Vector3d(2.0, 0.0, 0.0), isect.pos());
    EXPECT_EQ(Normal3d(1.0, 0.0, 0.0), isect.normal());
    EXPECT_EQ(8.0, tHit);

    EXPECT_FALSE(sp.intersect(
        Ray(Point3d(10.0, 0.0, 0.0), Vector3d(0.0, 1.0, 0.0)), &tHit, &isect));
}

TEST(SphereTest, AreaTest) {
    double rad = 2.0;
    double area = 4.0 * PI * rad * rad;
    Sphere sp0(Point3d(0.0, 0.0, 0.0), rad);
    EXPECT_EQ(area, sp0.area());

    Sphere sp1(Point3d(5.0, 4.0, 3.0), rad);
    EXPECT_EQ(area, sp1.area());
}

// ------------------------------
// Triangle class test
// ------------------------------
TEST(TriangleTest, InstanceTest) {
    Triangle t0;
    EXPECT_EQ_VEC(Point3d(), t0[0]);
    EXPECT_EQ_VEC(Point3d(), t0[1]);
    EXPECT_EQ_VEC(Point3d(), t0[2]);

    Triangle t1(Point3d(0, 0, 0),
                Point3d(0, 1, 0),
                Point3d(1, 1, 0));
    EXPECT_EQ_VEC(Point3d(0, 0, 0), t1[0]);
    EXPECT_EQ_VEC(Point3d(0, 1, 0), t1[1]);
    EXPECT_EQ_VEC(Point3d(1, 1, 0), t1[2]);
}

TEST(TriangleTest, IntersectionTest) {
    Triangle t0(Point3d(1, 0, 0),
                Point3d(0, 0, 0),
                Point3d(0, 1, 0));
    Ray ray = Ray(Point3d(0, 0, -1), (Vector3d(1, 1, 1) - Vector3d(0, 0, -1)).normalized());
    
    SurfaceInteraction isect;
    double tHit;
    EXPECT_TRUE(t0.intersect(ray));
    EXPECT_TRUE(t0.intersect(ray, &tHit, &isect));
    EXPECT_EQ(sqrt(6.0) / 2.0, tHit);

    ray = Ray(Point3d(-0.1, -0.1, 1.0), Vector3d(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray));
    EXPECT_FALSE(t0.intersect(ray, &tHit, &isect));

    ray = Ray(Point3d(0.6, 0.6, 1.0), Vector3d(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray));
    EXPECT_FALSE(t0.intersect(ray, &tHit, &isect));

    ray = Ray(Point3d(-0.1, 1.1, 1.0), Vector3d(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray));
    EXPECT_FALSE(t0.intersect(ray, &tHit, &isect));

    ray = Ray(Point3d(1.1, -0.1, 1.0), Vector3d(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray));
    EXPECT_FALSE(t0.intersect(ray, &tHit, &isect));
}

TEST(TriangleTest, AreaTest) {
    Triangle t0(Point3d(1, 0, 0),
                Point3d(0, 0, 0),
                Point3d(0, 1, 0));
    EXPECT_EQ(0.5, t0.area());
}

// ------------------------------
// Disk class test
// ------------------------------

/*
TEST(DiskTest, InstanceTest) {
    Disk disk;
    EXPECT_EQ_VEC(Point3d(), disk.center());
    EXPECT_EQ_VEC(Normal3d(), disk.normal());
    EXPECT_EQ(0.0, disk.radius());

    disk = Disk(Point(1.0, 0.0, 0.0), Normal(0.0, 1.0, 0.0), 1.0);
    EXPECT_EQ_VEC(Point(1.0, 0.0, 0.0), disk.center());
    EXPECT_EQ_VEC(Normal(0.0, 1.0, 0.0), disk.normal());
    EXPECT_EQ(1.0, disk.radius());
}

TEST(DiskTest, CopyConstructor) {
    Disk d0(Point(1.0, 0.0, 0.0), Normal(0.0, 1.0, 0.0), 1.0);
    Disk d1(d0);
    EXPECT_EQ_VEC(Point(1.0, 0.0, 0.0), d1.center());
    EXPECT_EQ_VEC(Normal(0.0, 1.0, 0.0), d1.normal());
    EXPECT_EQ(1.0, d1.radius());
}

TEST(DiskTest, IntersectionTest) {
    Disk disk(Point(1.0, 0.0, 0.0), Normal(0.0, 1.0, 0.0), 1.0);
    Hitpoint hitpoint;
    EXPECT_TRUE(disk.intersect(Ray(Point(1.0, 1.0, 0.0), Vector3d(0.0, -1.0, 0.0)), &hitpoint));
    EXPECT_EQ(1.0, hitpoint.distance());
}

TEST(DiskTest, AreaTest) {
    double rad = 5.0;
    double area = PI * rad * rad;
    Disk disk(Point(0.0, 0.0, 0.0), Normal(0.0, 1.0, 0.0), rad);
    EXPECT_EQ(area, disk.area());    
}
*/

// ------------------------------
// Bounds3d class test
// ------------------------------
TEST(Bounds3dTest, InstanceTest) {
    Bounds3d b;
    const double minValue = std::numeric_limits<double>::lowest();
    const double maxValue = std::numeric_limits<double>::max();
    EXPECT_EQ(Point3d(maxValue, maxValue, maxValue), b.posMin());
    EXPECT_EQ(Point3d(minValue, minValue, minValue), b.posMax());

    Bounds3d b0(Point3d(0.0, 0.0, 0.0), Point3d(1.0, 1.0, 1.0));
    EXPECT_EQ(Point3d(0.0, 0.0, 0.0), b0.posMin());
    EXPECT_EQ(Point3d(1.0, 1.0, 1.0), b0.posMax());
}

TEST(Bounds3dTest, CopyConstructor) {
    Bounds3d b0(Point3d(0.0, 0.0, 0.0), Point3d(1.0, 1.0, 1.0));
    Bounds3d b1(b0);
    EXPECT_EQ(Point3d(0.0, 0.0, 0.0), b1.posMin());
    EXPECT_EQ(Point3d(1.0, 1.0, 1.0), b1.posMax());
}

TEST(Bounds3dTest, MergeTest) {
    Bounds3d Bounds3d;
    Bounds3d.merge(Point3d(0.0, 0.0, 0.0));
    EXPECT_EQ(Point3d(0.0, 0.0, 0.0), Bounds3d.posMin());
    EXPECT_EQ(Point3d(0.0, 0.0, 0.0), Bounds3d.posMax());

    Bounds3d.merge(Point3d(1.0, 1.0, 1.0));
    EXPECT_EQ(Point3d(0.0, 0.0, 0.0), Bounds3d.posMin());
    EXPECT_EQ(Point3d(1.0, 1.0, 1.0), Bounds3d.posMax());
}

TEST(Bounds3dTest, IntersectionTest) {
    Bounds3d b0(Point3d(0.0, 0.0, 0.0), Point3d(1.0, 1.0, 1.0));
    double tMin, tMax;
    b0.intersect(Ray(Point3d(0.5, 0.5, -1.0), Vector3d(0.0, 0.0, 1.0)), &tMin, &tMax);
    EXPECT_DOUBLE_EQ(1.0, tMin);
    EXPECT_DOUBLE_EQ(2.0, tMax);
}
