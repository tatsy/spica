#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

inline void EXPECT_EQ_VEC(const Vector3& v1, const Vector3& v2) {
    EXPECT_EQ(v1.x(), v2.x());
    EXPECT_EQ(v1.y(), v2.y());
    EXPECT_EQ(v1.z(), v2.z());
}

// ------------------------------
// Primitive class test
// ------------------------------
TEST(PrimitiveTest, InstanceTest) {
    Primitive p;

    EXPECT_EQ(0.0, p.emission().x());
    EXPECT_EQ(0.0, p.emission().y());
    EXPECT_EQ(0.0, p.emission().z());

    EXPECT_EQ(0.0, p.color().x());
    EXPECT_EQ(0.0, p.color().y());
    EXPECT_EQ(0.0, p.color().z());

    EXPECT_EQ(REFLECTION_DIFFUSE, p.reftype());

    Ray ray;
    HitPoint hitpoint;
    ASSERT_DEATH(p.intersect(ray, hitpoint), "");

    Primitive q(Material(Color(0.25, 0.50, 0.75), Color(0.1, 0.2, 0.3), REFLECTION_REFRACTION));
    
    EXPECT_EQ(0.25, q.emission().x());
    EXPECT_EQ(0.50, q.emission().y());
    EXPECT_EQ(0.75, q.emission().z());

    EXPECT_EQ(0.1, q.color().x());
    EXPECT_EQ(0.2, q.color().y());
    EXPECT_EQ(0.3, q.color().z());

    EXPECT_EQ(REFLECTION_REFRACTION, q.reftype());

    p = q;

    EXPECT_EQ(0.25, p.emission().x());
    EXPECT_EQ(0.50, p.emission().y());
    EXPECT_EQ(0.75, p.emission().z());

    EXPECT_EQ(0.1, p.color().x());
    EXPECT_EQ(0.2, p.color().y());
    EXPECT_EQ(0.3, p.color().z());
    
    p = Primitive(q);

    EXPECT_EQ(0.25, p.emission().x());
    EXPECT_EQ(0.50, p.emission().y());
    EXPECT_EQ(0.75, p.emission().z());

    EXPECT_EQ(0.1, p.color().x());
    EXPECT_EQ(0.2, p.color().y());
    EXPECT_EQ(0.3, p.color().z());


}

// ------------------------------
// Plane class test
// ------------------------------
TEST(PlaneTest, InstanceTest) {
    Plane pl(3.0, Vector3(-1.0, 0.0, 0.0), Material(Color(0.1, 0.2, 0.3), Color(0.25, 0.50, 0.75), REFLECTION_DIFFUSE));

    EXPECT_EQ(3.0, pl.distance());

    EXPECT_EQ(-1.0, pl.normal().x());
    EXPECT_EQ(0.0, pl.normal().y());
    EXPECT_EQ(0.0, pl.normal().z());

    EXPECT_EQ(0.1, pl.emission().x());
    EXPECT_EQ(0.2, pl.emission().y());
    EXPECT_EQ(0.3, pl.emission().z());

    EXPECT_EQ(0.25, pl.color().x());
    EXPECT_EQ(0.50, pl.color().y());
    EXPECT_EQ(0.75, pl.color().z());

    HitPoint hitpoint;
    EXPECT_TRUE(pl.intersect(Ray(Vector3(0.0, 1.0, 1.0), Vector3(3.0, 4.0, 0.0).normalized()), hitpoint));

    EXPECT_EQ(3.0, hitpoint.position().x());
    EXPECT_EQ(5.0, hitpoint.position().y());
    EXPECT_EQ(1.0, hitpoint.position().z());

    EXPECT_EQ(5.0, hitpoint.distance());

    EXPECT_FALSE(pl.intersect(Ray(Vector3(0.0, 1.0, 1.0), Vector3(-1.0, 0.0, 0.0)), hitpoint));    
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

    Sphere sp(2.0, Vector3(0.0, 0.0, 0.0), Material(Color(), Color(0.75, 0.75, 0.75), REFLECTION_DIFFUSE));

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

    // copy constructor
    sp0 = sp;
    EXPECT_EQ(0.0, sp0.center().x());
    EXPECT_EQ(0.0, sp0.center().y());
    EXPECT_EQ(0.0, sp0.center().z());

    EXPECT_EQ(0.0, sp0.emission().x());
    EXPECT_EQ(0.0, sp0.emission().y());
    EXPECT_EQ(0.0, sp0.emission().z());

    EXPECT_EQ(0.75, sp0.color().x());
    EXPECT_EQ(0.75, sp0.color().y());
    EXPECT_EQ(0.75, sp0.color().z());

    // intersection
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

    double tHit;
    EXPECT_TRUE(t2.intersect(ray, &tHit));
    EXPECT_EQ(sqrt(6.0) / 2.0, tHit);
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
    Ray ray(Vector3(50.0, 40.8, 220.0), Vector3(-0.1, -0.1, -1.0).normalized());
    double tHit = INFTY;
    for (int i = 0; i < trimesh.numFaces(); i++) {
        Triangle tri = trimesh.getTriangle(i);
        double tTemp;
        if (tri.intersect(ray, &tTemp)) {
            if (tHit > tTemp) {
                tHit = tTemp;
            }
        }
    }
    trimesh.buildKdTreeAccel();
    HitPoint hitpoint;

    EXPECT_TRUE(trimesh.intersect(ray, hitpoint));
    EXPECT_EQ(tHit, hitpoint.distance());
}

