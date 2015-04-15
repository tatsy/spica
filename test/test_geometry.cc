#include "gtest/gtest.h"

#include "../include/rainy.h"
using namespace rainy;

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

    EXPECT_EQ(0, p.reftype());

    Ray ray;
    HitPoint hitpoint;
    ASSERT_DEATH(p.intersect(ray, hitpoint), "");

    Primitive q(Color(0.25, 0.50, 0.75), Color(0.1, 0.2, 0.3), REFLECTION_REFRACTION);
    
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
// Sphere class test
// ------------------------------
TEST(PlaneTest, InstanceTest) {
    Plane pl(3.0, Vector3(-1.0, 0.0, 0.0), Color(0.1, 0.2, 0.3), Color(0.25, 0.50, 0.75), REFLECTION_DIFFUSE);

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
    EXPECT_TRUE(pl.intersect(Ray(Vector3(0.0, 1.0, 1.0), Vector3(3.0, 4.0, 0.0).normalize()), hitpoint));

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