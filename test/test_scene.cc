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
        scene.add(Sphere(5.0, Vector3(0.0, 0.0, 0.0)), Material(Color(), Color(0.3, 0.5, 0.7), REFLECTION_DIFFUSE));
        scene.add(Sphere(2.0, Vector3(0.0, 10.0, 0.0)), Material(Color(15.0, 15.0, 15.0), Color(), REFLECTION_DIFFUSE), true);
    }

    virtual void TearDown() {
    }

protected:
    Scene scene;
};

TEST_F(SceneTest, InstanceTest) {
    EXPECT_EQ(2, scene.numObjects());
    EXPECT_EQ(1, scene.lightID());
}

TEST_F(SceneTest, IntersectionTest) {
    Intersection isect;
    Ray ray(Vector3(0.0, 0.0, 10.0), Vector3(0.0, 0.0, -1.0));
    EXPECT_TRUE(scene.intersect(ray, isect));

    Hitpoint hitpoint = isect.hitpoint();
    EXPECT_EQ(0, isect.objectId());
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 5.0), hitpoint.position());

    ray = Ray(Vector3(0.0, 0.0, 10.0), Vector3(0.0, 1.0, 0.0));
    EXPECT_FALSE(scene.intersect(ray, isect));
}
