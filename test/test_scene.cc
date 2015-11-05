#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

class SceneTest : public ::testing::Test {
protected:
    SceneTest() {}

    virtual ~SceneTest() {}

    virtual void SetUp() {
        nTrial = 100;
        s1 = Sphere(Vector3D(0.0, 0.0, 0.0), 5.0);
        s2 = Sphere(Vector3D(0.0, 10.0, 0.0), 2.0);
        scene.addShape(s1, LambertianBRDF::factory(Color(0.3, 0.5, 0.7)));
        scene.setAreaLight(s2, Color(32.0, 32.0, 32.0));
    }

    virtual void TearDown() {
    }

protected:
    Scene scene;
    Sphere s1;
    Sphere s2;
    int nTrial;
};

TEST_F(SceneTest, InstanceTest) {
    EXPECT_GT(scene.numTriangles(), 0);
}

TEST_F(SceneTest, AccelNotPrepared) {
    Intersection isect;
    Ray ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 0.0, -1.0));
    ASSERT_DEATH(scene.intersect(ray, &isect), "");
}

TEST_F(SceneTest, BoundingSphere) {
    Camera cam = Camera::perspective(Vector3D{0.0, 0.0, 100.0},
                                     Vector3D{0.0, 0.0, -1.0},
                                     Vector3D{0.0, 1.0, 0.0},
                                     45.0,
                                     800,
                                     600,
                                     1.0);
    Sphere bounds = scene.boundingSphere(cam);

    EXPECT_LE((s1.center() - bounds.center()).norm(), bounds.radius() - s1.radius() + EPS);
    EXPECT_LE((s2.center() - bounds.center()).norm(), bounds.radius() - s2.radius() + EPS);
}

TEST_F(SceneTest, IsTextured) {
    Trimesh mesh(kDataDirectory + "tex_cube.ply");
    int curTris = scene.numTriangles();
    scene.addShape(mesh, LambertianBRDF::factory(Color(0.5, 0.5, 0.5)));
    for (int i = curTris; i < scene.numTriangles(); i++) {
        EXPECT_TRUE(scene.isTextured(i));
    }
}

TEST_F(SceneTest, BBVHIntersectionTest) {
    scene.setAccelType(AccelType::BBVH);
    scene.computeAccelerator();

    Intersection isect;
    Ray ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 0.0, -1.0).normalized());
    EXPECT_TRUE(scene.intersect(ray, &isect));

    EXPECT_NE(isect.objectID(), -1);
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 5.0), isect.position());

    ray = Ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 1.0, 0.0));
    EXPECT_FALSE(scene.intersect(ray, &isect));
    EXPECT_EQ(isect.objectID(), -1);
}

TEST_F(SceneTest, QBVHIntersectionTest) {
    scene.setAccelType(AccelType::QBVH);
    scene.computeAccelerator();

    Intersection isect;
    Ray ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 0.0, -1.0).normalized());
    EXPECT_TRUE(scene.intersect(ray, &isect));

    EXPECT_NE(isect.objectID(), -1);
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 5.0), isect.position());

    ray = Ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 1.0, 0.0));
    EXPECT_FALSE(scene.intersect(ray, &isect));
    EXPECT_EQ(isect.objectID(), -1);
}

TEST_F(SceneTest, KdTreeIntersectionTest) {
    scene.setAccelType(AccelType::KdTree);
    scene.computeAccelerator();

    Intersection isect;
    Ray ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 0.0, -1.0));
    EXPECT_TRUE(scene.intersect(ray, &isect));

    EXPECT_NE(isect.objectID(), -1);
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 5.0), isect.position());

    ray = Ray(Vector3D(0.0, 0.0, 10.0), Vector3D(0.0, 1.0, 0.0));
    EXPECT_FALSE(scene.intersect(ray, &isect));
    EXPECT_EQ(isect.objectID(), -1);
}

TEST_F(SceneTest, QBVHvsKdTree) {
    Scene scene1, scene2;
    Camera cam1, cam2;
    cornellBox(&scene1, &cam1, 400, 300);
    cornellBox(&scene2, &cam2, 400, 300);
    scene1.setAccelType(AccelType::QBVH);
    scene1.computeAccelerator();
    scene2.setAccelType(AccelType::KdTree);
    scene2.computeAccelerator();

    Random rng = Random((unsigned int)time(NULL));

    for (int i = 0; i < nTrial; i++) {
        Vector3D from  = Vector3D(rng.nextReal(), rng.nextReal(), rng.nextReal()) * 20.0 - Vector3D(10.0, 10.0, 10.0);
        Vector3D to    = Vector3D(rng.nextReal(), rng.nextReal(), rng.nextReal()) * 20.0 - Vector3D(10.0, 10.0, 10.0);
        Vector3D dir = (to - from).normalized();
        Ray ray(from, dir);

        Intersection isect1, isect2;
        EXPECT_EQ(scene1.intersect(ray, &isect1), scene2.intersect(ray, &isect2));
        EXPECT_EQ(isect1.objectID(), isect2.objectID());
        EXPECT_NEAR(isect1.distance(), isect2.distance(), 1.0e-4);
    }
}

TEST_F(SceneTest, BBVHvsQBVH) {
    Scene scene1, scene2;
    Camera cam1, cam2;
    cornellBox(&scene1, &cam1, 400, 300);
    cornellBox(&scene2, &cam2, 400, 300);
    scene1.setAccelType(AccelType::BBVH);
    scene1.computeAccelerator();
    scene2.setAccelType(AccelType::QBVH);
    scene2.computeAccelerator();

    Random rng = Random();

    for (int i = 0; i < nTrial; i++) {
        Vector3D from  = Vector3D(rng.nextReal(), rng.nextReal(), rng.nextReal()) * 20.0 - Vector3D(10.0, 10.0, 10.0);
        Vector3D to    = Vector3D(rng.nextReal(), rng.nextReal(), rng.nextReal()) * 20.0 - Vector3D(10.0, 10.0, 10.0);
        Vector3D dir = (to - from).normalized();
        Ray ray(from, dir);

        Intersection isect1, isect2;
        EXPECT_EQ(scene1.intersect(ray, &isect1), scene2.intersect(ray, &isect2));
        EXPECT_EQ(isect1.objectID(), isect2.objectID());
        EXPECT_NEAR(isect1.distance(), isect2.distance(), 1.0e-4);
    }
}
