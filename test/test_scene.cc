#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

class SceneTest : public ::testing::Test {
protected:
    SceneTest()
        : scene()
    {
    }

    ~SceneTest()
    {
    }

    virtual void SetUp() {
        scene.add(Sphere(5.0, Vector3D(0.0, 0.0, 0.0)), LambertianBRDF::factory(Color(0.3, 0.5, 0.7)));
        scene.add(Sphere(2.0, Vector3D(0.0, 10.0, 0.0)), LambertianBRDF::factory(Color(0.0, 0.0, 0.0), Color(32.0, 32.0, 32.0)), true);
    }

    virtual void TearDown() {
    }

protected:
    Scene scene;
};

TEST_F(SceneTest, InstanceTest) {
    EXPECT_GT(scene.numTriangles(), 0);
}

TEST_F(SceneTest, AccelNotPrepared) {
    Intersection isect;
    Ray ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 0.0, -1.0));
    ASSERT_DEATH(scene.intersect(ray, isect), "");
}

TEST_F(SceneTest, IntersectionTest) {
    scene.setAccelerator();

    Intersection isect;
    Ray ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 0.0, -1.0));
    EXPECT_TRUE(scene.intersect(ray, isect));

    Hitpoint hitpoint = isect.hitpoint();
    EXPECT_NE(isect.objectId(), -1);
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 5.0), hitpoint.position());

    ray = Ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 1.0, 0.0));
    EXPECT_FALSE(scene.intersect(ray, isect));
    EXPECT_EQ(isect.objectId(), -1);
}
