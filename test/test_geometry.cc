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

    // copy constructor
    sp0 = sp;
    EXPECT_EQ(0.0, sp0.center().x());
    EXPECT_EQ(0.0, sp0.center().y());
    EXPECT_EQ(0.0, sp0.center().z());

    // intersection
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

    Triangle t2(Vector3(1, 0, 0),
                Vector3(0, 0, 0),
                Vector3(0, 1, 0));
    Ray ray(Vector3(0, 0, -1), (Vector3(1, 1, 1) - Vector3(0, 0, -1)).normalized());

    Hitpoint hitpoint;
    EXPECT_TRUE(t2.intersect(ray, &hitpoint));
    EXPECT_EQ(sqrt(6.0) / 2.0, hitpoint.distance());
}

TEST(BBoxTest, InstanceTest) {
    BBox b0(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 0.0), b0.posMin());
    EXPECT_EQ_VEC(Vector3(1.0, 1.0, 1.0), b0.posMax());

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

