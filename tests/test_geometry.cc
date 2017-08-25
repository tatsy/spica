#include "gtest/gtest.h"

#include <cmath>

#include "spica.h"
using namespace spica;

// ------------------------------
// Sphere class test
// ------------------------------

TEST(BSphereTest, InstanceTest) {
    BSphere sp0;
    EXPECT_EQ(0.0, sp0.center().x());
    EXPECT_EQ(0.0, sp0.center().y());
    EXPECT_EQ(0.0, sp0.center().z());
    EXPECT_EQ(0.0, sp0.radius());

    BSphere sp(Point3d(0.0, 0.0, 0.0), 2.0);
    EXPECT_EQ(0.0, sp.center().x());
    EXPECT_EQ(0.0, sp.center().y());
    EXPECT_EQ(0.0, sp.center().z());
}

TEST(SphereTest, CopyConstructor) {
    BSphere sp(Point3d(0.0, 0.0, 0.0), 2.0);
    BSphere sp0 = sp;
    EXPECT_EQ(0.0, sp0.center().x());
    EXPECT_EQ(0.0, sp0.center().y());
    EXPECT_EQ(0.0, sp0.center().z());
}

// ------------------------------
// Triangle class test
// ------------------------------
TEST(TriangleTest, InstanceTest) {
    Triangle t0;
    EXPECT_EQ(Point3d(), t0[0]);
    EXPECT_EQ(Point3d(), t0[1]);
    EXPECT_EQ(Point3d(), t0[2]);

    Triangle t1(Point3d(0, 0, 0),
                Point3d(0, 1, 0),
                Point3d(1, 1, 0));
    EXPECT_EQ(Point3d(0, 0, 0), t1[0]);
    EXPECT_EQ(Point3d(0, 1, 0), t1[1]);
    EXPECT_EQ(Point3d(1, 1, 0), t1[2]);
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
