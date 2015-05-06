#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

inline void EXPECT_EQ_VEC(const Vector3& v1, const Vector3& v2) {
    EXPECT_EQ(v1.x(), v2.x());
    EXPECT_EQ(v1.y(), v2.y());
    EXPECT_EQ(v1.z(), v2.z());
}

// ------------------------------
// Plane class test
// ------------------------------
TEST(PlaneTest, InstanceTest) {
    Plane pl(3.0, Vector3(-1.0, 0.0, 0.0));

    EXPECT_EQ(3.0, pl.distance());

    EXPECT_EQ(-1.0, pl.normal().x());
    EXPECT_EQ(0.0, pl.normal().y());
    EXPECT_EQ(0.0, pl.normal().z());

    Hitpoint hitpoint;
    EXPECT_TRUE(pl.intersect(Ray(Vector3(0.0, 1.0, 1.0), Vector3(3.0, 4.0, 0.0).normalized()), &hitpoint));
    EXPECT_EQ(5.0, hitpoint.distance());
    EXPECT_FALSE(pl.intersect(Ray(Vector3(0.0, 1.0, 1.0), Vector3(-1.0, 0.0, 0.0)), &hitpoint));    
}

// ------------------------------
// Sphere class test
// ------------------------------
TEST(SphereTest, InstanceTest) {
    Sphere sp0;
    EXPECT_EQ(0.0, sp0.center().x());
    EXPECT_EQ(0.0, sp0.center().y());
    EXPECT_EQ(0.0, sp0.center().z());
    EXPECT_EQ(0.0, sp0.radius());

    Sphere sp(2.0, Vector3(0.0, 0.0, 0.0));
    EXPECT_EQ(0.0, sp.center().x());
    EXPECT_EQ(0.0, sp.center().y());
    EXPECT_EQ(0.0, sp.center().z());
}

TEST(SphereTest, CopyConstructor) {
    Sphere sp(2.0, Vector3(0.0, 0.0, 0.0));
    Sphere sp0 = sp;
    EXPECT_EQ(0.0, sp0.center().x());
    EXPECT_EQ(0.0, sp0.center().y());
    EXPECT_EQ(0.0, sp0.center().z());
}

TEST(SphereTest, IntersectionTest) {
    Sphere sp(2.0, Vector3(0.0, 0.0, 0.0));
    Hitpoint hitpoint;
    EXPECT_TRUE(sp.intersect(Ray(Vector3(10.0, 0.0, 0.0), Vector3(-1.0, 0.0, 0.0)), &hitpoint));
    EXPECT_EQ(2.0, hitpoint.position().x());
    EXPECT_EQ(0.0, hitpoint.position().y());
    EXPECT_EQ(0.0, hitpoint.position().z());

    EXPECT_EQ(1.0, hitpoint.normal().x());
    EXPECT_EQ(0.0, hitpoint.normal().y());
    EXPECT_EQ(0.0, hitpoint.normal().z());

    EXPECT_FALSE(sp.intersect(Ray(Vector3(10.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)), &hitpoint));
}

TEST(SphereTest, AreaTest) {
    double rad = 2.0;
    double area = 4.0 * PI * rad * rad;
    Sphere sp0(rad, Vector3(0.0, 0.0, 0.0));
    EXPECT_EQ(area, sp0.area());

    Sphere sp1(rad, Vector3(5.0, 4.0, 3.0));
    EXPECT_EQ(area, sp1.area());
}

// ------------------------------
// Triangle class test
// ------------------------------
TEST(TriangleTest, InstanceTest) {
    Triangle t0;
    EXPECT_EQ_VEC(Vector3(), t0.p0());
    EXPECT_EQ_VEC(Vector3(), t0.p1());
    EXPECT_EQ_VEC(Vector3(), t0.p2());

    Triangle t1(Vector3(1, 2, 3),
                Vector3(2, 3, 4),
                Vector3(3, 4, 5));
    EXPECT_EQ_VEC(Vector3(1, 2, 3), t1.p0());
    EXPECT_EQ_VEC(Vector3(2, 3, 4), t1.p1());
    EXPECT_EQ_VEC(Vector3(3, 4, 5), t1.p2());
    EXPECT_EQ_VEC(Vector3(1, 2, 3), t1.p(0));
    EXPECT_EQ_VEC(Vector3(2, 3, 4), t1.p(1));
    EXPECT_EQ_VEC(Vector3(3, 4, 5), t1.p(2));
}

TEST(TriangleTest, IntersectionTest) {
    Triangle t0(Vector3(1, 0, 0),
                Vector3(0, 0, 0),
                Vector3(0, 1, 0));
    Ray ray;
    Hitpoint hitpoint;

    ray = Ray(Vector3(0, 0, -1), (Vector3(1, 1, 1) - Vector3(0, 0, -1)).normalized());
    EXPECT_TRUE(t0.intersect(ray, &hitpoint));
    EXPECT_EQ(sqrt(6.0) / 2.0, hitpoint.distance());

    ray = Ray(Vector3(-0.1, -0.1, 1.0), Vector3(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray, &hitpoint));

    ray = Ray(Vector3(0.6, 0.6, 1.0), Vector3(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray, &hitpoint));

    ray = Ray(Vector3(-0.1, 1.1, 1.0), Vector3(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray, &hitpoint));

    ray = Ray(Vector3(1.1, -0.1, 1.0), Vector3(0.0, 0.0, -1.0));
    EXPECT_FALSE(t0.intersect(ray, &hitpoint));
}

TEST(TriangleTest, AreaTest) {
    Triangle t0(Vector3(1, 0, 0),
                Vector3(0, 0, 0),
                Vector3(0, 1, 0));
    EXPECT_EQ(0.5, t0.area());
}

// ------------------------------
// Quad class test
// ------------------------------
TEST(QuadTest, InstanceTest) {
    Quad quad;
    EXPECT_EQ_VEC(Vector3(), quad.p0());
    EXPECT_EQ_VEC(Vector3(), quad.p1());
    EXPECT_EQ_VEC(Vector3(), quad.p2());
    EXPECT_EQ_VEC(Vector3(), quad.p3());
    ASSERT_DEATH(quad.normal(), "");

    quad = Quad(Vector3(-1.0, -1.0, 0.0),
                Vector3( 1.0, -1.0, 0.0),
                Vector3( 1.0,  1.0, 0.0),
                Vector3(-1.0,  1.0, 0.0));
    EXPECT_EQ_VEC(Vector3(-1.0, -1.0, 0.0), quad.p0());
    EXPECT_EQ_VEC(Vector3( 1.0, -1.0, 0.0), quad.p1());
    EXPECT_EQ_VEC(Vector3( 1.0,  1.0, 0.0), quad.p2());
    EXPECT_EQ_VEC(Vector3(-1.0,  1.0, 0.0), quad.p3());
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 1.0), quad.normal());
}

TEST(QuadTest, CopyConstructor) {
    Quad q0(Vector3(-1.0, -1.0, 0.0),
            Vector3( 1.0, -1.0, 0.0),
            Vector3( 1.0,  1.0, 0.0),
            Vector3(-1.0,  1.0, 0.0));

    Quad quad(q0);
    EXPECT_EQ_VEC(Vector3(-1.0, -1.0, 0.0), quad.p0());
    EXPECT_EQ_VEC(Vector3( 1.0, -1.0, 0.0), quad.p1());
    EXPECT_EQ_VEC(Vector3( 1.0,  1.0, 0.0), quad.p2());
    EXPECT_EQ_VEC(Vector3(-1.0,  1.0, 0.0), quad.p3());
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 1.0), quad.normal());
}
 
TEST(QuadTest, IntersectionTest) {
    Quad quad(Vector3(-1.0, -1.0, 0.0),
              Vector3( 1.0, -1.0, 0.0),
              Vector3( 1.0,  1.0, 0.0),
              Vector3(-1.0,  1.0, 0.0));

    Hitpoint hitpoint;

    // Hit in the center
    EXPECT_TRUE(quad.intersect(Ray(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_EQ(1.0, hitpoint.distance());

    // Hit on the lim
    EXPECT_TRUE(quad.intersect(Ray(Vector3(-1.0, -1.0, 1.0), Vector3(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_EQ(1.0, hitpoint.distance());
    EXPECT_TRUE(quad.intersect(Ray(Vector3(1.0, 1.0, 1.0), Vector3(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_EQ(1.0, hitpoint.distance());

    // Not hit
    EXPECT_FALSE(quad.intersect(Ray(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, 1.0)), &hitpoint));
    EXPECT_FALSE(quad.intersect(Ray(Vector3(-1.1, -1.1, 1.0), Vector3(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_FALSE(quad.intersect(Ray(Vector3(-1.1,  1.1, 1.0), Vector3(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_FALSE(quad.intersect(Ray(Vector3( 1.1, -1.1, 1.0), Vector3(0.0, 0.0, -1.0)), &hitpoint));
    EXPECT_FALSE(quad.intersect(Ray(Vector3( 1.1,  1.1, 1.0), Vector3(0.0, 0.0, -1.0)), &hitpoint));
}

TEST(QuadTest, RandomIntersection) {
    Quad quad(Vector3(-1.0, -1.0, 0.0),
              Vector3(1.0, -1.0, 0.0),
              Vector3(1.0, 1.0, 0.0),
              Vector3(-1.0, 1.0, 0.0));

    Random rng = Random::getRNG();

    for (int i = 0; i < 100; i++) {
        double tx = rng.randReal() * 4.0 - 2.0;
        double ty = rng.randReal() * 4.0 - 2.0;
        double fx = rng.randReal() * 4.0 - 2.0;
        double fy = rng.randReal() * 4.0 - 2.0;
        Vector3 from(fx, fy, -1.0);
        Vector3 to(tx, ty, 0.0);
        Vector3 dir = (to - from).normalized();
        Ray ray(from, dir);
        Hitpoint hitpoint;
        if (abs(tx) <= 1.0 && abs(ty) <= 1.0) {
            EXPECT_TRUE(quad.intersect(ray, &hitpoint));
        } else {
            EXPECT_FALSE(quad.intersect(ray, &hitpoint));
        }
    }
}

TEST(QuadTest, AreaTest) {
    Quad q0(Vector3(-1.0, -1.0, 0.0),
            Vector3( 1.0, -1.0, 0.0),
            Vector3( 1.0,  1.0, 0.0),
            Vector3(-1.0,  1.0, 0.0));
    EXPECT_EQ(4.0, q0.area());
}

// ------------------------------
// Disk class test
// ------------------------------
TEST(DiskTest, InstanceTest) {
    Disk disk;
    EXPECT_EQ_VEC(Vector3(), disk.center());
    EXPECT_EQ_VEC(Vector3(), disk.normal());
    EXPECT_EQ(0.0, disk.radius());

    disk = Disk(Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 1.0);
    EXPECT_EQ_VEC(Vector3(1.0, 0.0, 0.0), disk.center());
    EXPECT_EQ_VEC(Vector3(0.0, 1.0, 0.0), disk.normal());
    EXPECT_EQ(1.0, disk.radius());
}

TEST(DiskTest, CopyConstructor) {
    Disk d0(Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 1.0);
    Disk d1(d0);
    EXPECT_EQ_VEC(Vector3(1.0, 0.0, 0.0), d1.center());
    EXPECT_EQ_VEC(Vector3(0.0, 1.0, 0.0), d1.normal());
    EXPECT_EQ(1.0, d1.radius());
}

TEST(DiskTest, IntersectionTest) {
    Disk disk(Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 1.0);
    Hitpoint hitpoint;
    EXPECT_TRUE(disk.intersect(Ray(Vector3(1.0, 1.0, 0.0), Vector3(0.0, -1.0, 0.0)), &hitpoint));
    EXPECT_EQ(1.0, hitpoint.distance());
}

TEST(DiskTest, AreaTest) {
    double rad = 5.0;
    double area = PI * rad * rad;
    Disk disk(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), rad);
    EXPECT_EQ(area, disk.area());    
}

// ------------------------------
// BBox class test
// ------------------------------
TEST(BBoxTest, InstanceTest) {
    BBox b0(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 0.0), b0.posMin());
    EXPECT_EQ_VEC(Vector3(1.0, 1.0, 1.0), b0.posMax());
}

TEST(BBoxTest, CopyConstructor) {
    BBox b0(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    BBox b1(b0);
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 0.0), b1.posMin());
    EXPECT_EQ_VEC(Vector3(1.0, 1.0, 1.0), b1.posMax());
}

TEST(BBoxTest, IntersectionTest) {
    BBox b0(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    double tMin, tMax;
    b0.intersect(Ray(Vector3(0.5, 0.5, -1.0), Vector3(0.0, 0.0, 1.0)), &tMin, &tMax);
    EXPECT_EQ(1.0, tMin);
    EXPECT_EQ(2.0, tMax);
}

// ------------------------------
// Trimesh class test
// ------------------------------
TEST(TrimeshTest, InstanceTest) {
    Trimesh trimesh(DATA_DIR + "bunny.ply");
    Ray ray(Vector3(0.0, 0.8, 100.0), Vector3(0.0, 0.0, -1.0).normalized());

    Hitpoint hpGT;
    for (int i = 0; i < trimesh.numFaces(); i++) {
        Triangle tri = trimesh.getTriangle(i);
        Hitpoint hpTemp;
        if (tri.intersect(ray, &hpTemp)) {
            if (hpGT.distance() > hpTemp.distance()) {
                hpGT = hpTemp;
            }
        }
    }
    trimesh.buildKdTreeAccel();
    Hitpoint hitpoint;

    EXPECT_TRUE(trimesh.intersect(ray, &hitpoint));
    EXPECT_EQ(hpGT.distance(), hitpoint.distance());
}

