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

    Sphere sp(Point(0.0, 0.0, 0.0), 2.0);
    EXPECT_EQ(0.0, sp.center().x());
    EXPECT_EQ(0.0, sp.center().y());
    EXPECT_EQ(0.0, sp.center().z());
}

TEST(SphereTest, CopyConstructor) {
    Sphere sp(Point(0.0, 0.0, 0.0), 2.0);
    Sphere sp0 = sp;
    EXPECT_EQ(0.0, sp0.center().x());
    EXPECT_EQ(0.0, sp0.center().y());
    EXPECT_EQ(0.0, sp0.center().z());
}

TEST(SphereTest, IntersectionTest) {
    Sphere sp(Point(0.0, 0.0, 0.0), 2.0);
    SurfaceInteraction isect;
    double tHit;
    EXPECT_TRUE(sp.intersect(
        Ray(Point(10.0, 0.0, 0.0), Vector3D(-1.0, 0.0, 0.0)), &tHit, &isect));
    EXPECT_EQ(Vector3D(2.0, 0.0, 0.0), isect.pos());
    EXPECT_EQ(Normal3D(1.0, 0.0, 0.0), isect.normal());
    EXPECT_EQ(8.0, tHit);

    EXPECT_FALSE(sp.intersect(
        Ray(Point(10.0, 0.0, 0.0), Vector3D(0.0, 1.0, 0.0)), &tHit, &isect));
}

TEST(SphereTest, AreaTest) {
    double rad = 2.0;
    double area = 4.0 * PI * rad * rad;
    Sphere sp0(Point(0.0, 0.0, 0.0), rad);
    EXPECT_EQ(area, sp0.area());

    Sphere sp1(Point(5.0, 4.0, 3.0), rad);
    EXPECT_EQ(area, sp1.area());
}

// ------------------------------
// Triangle class test
// ------------------------------
TEST(TriangleTest, InstanceTest) {
    Triangle t0;
    EXPECT_EQ_VEC(Point(), t0[0]);
    EXPECT_EQ_VEC(Point(), t0[1]);
    EXPECT_EQ_VEC(Point(), t0[2]);

    Triangle t1(Point(0, 0, 0),
                Point(0, 1, 0),
                Point(1, 1, 0));
    EXPECT_EQ_VEC(Point(0, 0, 0), t1[0]);
    EXPECT_EQ_VEC(Point(0, 1, 0), t1[1]);
    EXPECT_EQ_VEC(Point(1, 1, 0), t1[2]);
}

/*
TEST(TriangleTest, IntersectionTest) {
    Triangle t0(Point(1, 0, 0),
                Point(0, 0, 0),
                Point(0, 1, 0));
    Ray ray;
    Hitpoint hitpoint;

    ray = Ray(Point(0, 0, -1), (Vector3D(1, 1, 1) - Vector3D(0, 0, -1)).normalized());
    EXPECT_TRUE(t0.intersect(ray, &hitpoint));
    EXPECT_EQ(sqrt(6.0) / 2.0, hitpoint.distance());

    ray = Ray(Point(-0.1, -0.1, 1.0), Vector3D(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray, &hitpoint));

    ray = Ray(Point(0.6, 0.6, 1.0), Vector3D(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray, &hitpoint));

    ray = Ray(Point(-0.1, 1.1, 1.0), Vector3D(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray, &hitpoint));

    ray = Ray(Point(1.1, -0.1, 1.0), Vector3D(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray, &hitpoint));
}

TEST(TriangleTest, AreaTest) {
    Triangle t0(Point(1, 0, 0),
                Point(0, 0, 0),
                Point(0, 1, 0));
    EXPECT_EQ(0.5, t0.area());
}

// ------------------------------
// Quad class test
// ------------------------------
TEST(QuadTest, InstanceTest) {
    Quad quad;
    EXPECT_EQ_VEC(Point(), quad.get(0));
    EXPECT_EQ_VEC(Point(), quad.get(1));
    EXPECT_EQ_VEC(Point(), quad.get(2));
    EXPECT_EQ_VEC(Point(), quad.get(3));
    ASSERT_DEATH(quad.normal(), "");

    quad = Quad(Point(-1.0, -1.0, 0.0),
                Point( 1.0, -1.0, 0.0),
                Point( 1.0,  1.0, 0.0),
                Point(-1.0,  1.0, 0.0));
    EXPECT_EQ_VEC(Point(-1.0, -1.0, 0.0), quad[0]);
    EXPECT_EQ_VEC(Point( 1.0, -1.0, 0.0), quad[1]);
    EXPECT_EQ_VEC(Point( 1.0,  1.0, 0.0), quad[2]);
    EXPECT_EQ_VEC(Point(-1.0,  1.0, 0.0), quad[3]);
    EXPECT_EQ_VEC(Normal(0.0, 0.0, 1.0), quad.normal());
}

TEST(QuadTest, CopyConstructor) {
    Quad q0(Point(-1.0, -1.0, 0.0),
            Point( 1.0, -1.0, 0.0),
            Point( 1.0,  1.0, 0.0),
            Point(-1.0,  1.0, 0.0));

    Quad quad(q0);
    EXPECT_EQ_VEC(Point(-1.0, -1.0, 0.0), quad[0]);
    EXPECT_EQ_VEC(Point( 1.0, -1.0, 0.0), quad[1]);
    EXPECT_EQ_VEC(Point( 1.0,  1.0, 0.0), quad[2]);
    EXPECT_EQ_VEC(Point(-1.0,  1.0, 0.0), quad[3]);
    EXPECT_EQ_VEC(Normal(0.0, 0.0, 1.0), quad.normal());
}
 
TEST(QuadTest, IntersectionTest) {
    Quad quad(Point(-1.0, -1.0, 0.0),
              Point( 1.0, -1.0, 0.0),
              Point( 1.0,  1.0, 0.0),
              Point(-1.0,  1.0, 0.0));

    Hitpoint hitpoint;

    // Hit in the center
    EXPECT_TRUE(quad.intersect(Ray(Point(0.0, 0.0, 1.0), Vector3D(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_EQ(1.0, hitpoint.distance());

    // Hit on the lim
    EXPECT_TRUE(quad.intersect(Ray(Point(-1.0, -1.0, 1.0), Vector3D(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_EQ(1.0, hitpoint.distance());
    EXPECT_TRUE(quad.intersect(Ray(Point(1.0, 1.0, 1.0), Vector3D(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_EQ(1.0, hitpoint.distance());

    // Not hit
    EXPECT_FALSE(quad.intersect(Ray(Point(0.0, 0.0, 1.0), Vector3D(0.0, 0.0, 1.0)), &hitpoint));
    EXPECT_FALSE(quad.intersect(Ray(Point(-1.1, -1.1, 1.0), Vector3D(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_FALSE(quad.intersect(Ray(Point(-1.1,  1.1, 1.0), Vector3D(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_FALSE(quad.intersect(Ray(Point( 1.1, -1.1, 1.0), Vector3D(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_FALSE(quad.intersect(Ray(Point( 1.1,  1.1, 1.0), Vector3D(0.0, 0.0, -1.0)), &hitpoint));
}

TEST(QuadTest, RandomIntersection) {
    Quad quad(Point(-1.0, -1.0, 0.0),
              Point(1.0, -1.0, 0.0),
              Point(1.0, 1.0, 0.0),
              Point(-1.0, 1.0, 0.0));

    Random rng = Random();

    for (int i = 0; i < 100; i++) {
        double tx = rng.nextReal() * 4.0 - 2.0;
        double ty = rng.nextReal() * 4.0 - 2.0;
        double fx = rng.nextReal() * 4.0 - 2.0;
        double fy = rng.nextReal() * 4.0 - 2.0;
        Point from(fx, fy, -1.0);
        Point to(tx, ty, 0.0);
        Vector3D dir = (to - from).normalized();
        double dist = (to - from).norm();
        Ray ray(from, dir);
        Hitpoint hitpoint;
        if (std::abs(tx) <= 1.0 && std::abs(ty) <= 1.0) {
            EXPECT_TRUE(quad.intersect(ray, &hitpoint));
            EXPECT_NEAR(dist, hitpoint.distance(), 1.0e-8);
        } else {
            EXPECT_FALSE(quad.intersect(ray, &hitpoint));
        }
    }
}

TEST(QuadTest, AreaTest) {
    Quad q0(Point(-1.0, -1.0, 0.0),
            Point( 1.0, -1.0, 0.0),
            Point( 1.0,  1.0, 0.0),
            Point(-1.0,  1.0, 0.0));
    EXPECT_EQ(4.0, q0.area());
}

// ------------------------------
// Disk class test
// ------------------------------

TEST(DiskTest, InstanceTest) {
    Disk disk;
    EXPECT_EQ_VEC(Point(), disk.center());
    EXPECT_EQ_VEC(Normal(), disk.normal());
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
    EXPECT_TRUE(disk.intersect(Ray(Point(1.0, 1.0, 0.0), Vector3D(0.0, -1.0, 0.0)), &hitpoint));
    EXPECT_EQ(1.0, hitpoint.distance());
}

TEST(DiskTest, AreaTest) {
    double rad = 5.0;
    double area = PI * rad * rad;
    Disk disk(Point(0.0, 0.0, 0.0), Normal(0.0, 1.0, 0.0), rad);
    EXPECT_EQ(area, disk.area());    
}

// ------------------------------
// BBox class test
// ------------------------------
TEST(BBoxTest, InstanceTest) {
    BBox b;
    EXPECT_EQ_VEC(Point(INFTY, INFTY, INFTY), b.posMin());
    EXPECT_EQ_VEC(Point(-INFTY, -INFTY, -INFTY), b.posMax());

    BBox b0(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    EXPECT_EQ_VEC(Point(0.0, 0.0, 0.0), b0.posMin());
    EXPECT_EQ_VEC(Point(1.0, 1.0, 1.0), b0.posMax());
}

TEST(BBoxTest, CopyConstructor) {
    BBox b0(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    BBox b1(b0);
    EXPECT_EQ_VEC(Point(0.0, 0.0, 0.0), b1.posMin());
    EXPECT_EQ_VEC(Point(1.0, 1.0, 1.0), b1.posMax());
}

TEST(BBoxTest, InsideTest) {
    BBox b0(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    EXPECT_TRUE(b0.inside(Point(0.5, 0.5, 0.5)));
    EXPECT_TRUE(b0.inside(Point(0.0, 0.0, 0.0)));
    EXPECT_TRUE(b0.inside(Point(1.0, 1.0, 1.0)));
    EXPECT_FALSE(b0.inside(Point(1.0, 1.0, 1.5)));
}

TEST(BBoxTest, MergeTest) {
    BBox bbox;
    bbox.merge(Point(0.0, 0.0, 0.0));
    EXPECT_EQ_VEC(Point(0.0, 0.0, 0.0), bbox.posMin());
    EXPECT_EQ_VEC(Point(0.0, 0.0, 0.0), bbox.posMax());

    bbox.merge(Point(1.0, 1.0, 1.0));
    EXPECT_EQ_VEC(Point(0.0, 0.0, 0.0), bbox.posMin());
    EXPECT_EQ_VEC(Point(1.0, 1.0, 1.0), bbox.posMax());
}

TEST(BBoxTest, IntersectionTest) {
    BBox b0(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    double tMin, tMax;
    b0.intersect(Ray(Point(0.5, 0.5, -1.0), Vector3D(0.0, 0.0, 1.0)), &tMin, &tMax);
    EXPECT_EQ(1.0, tMin);
    EXPECT_EQ(2.0, tMax);
}
*/
