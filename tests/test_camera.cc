#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

// -----------------------------------------------------------------------------
// DoF Camera class test
// -----------------------------------------------------------------------------

class DoFCameraTest : public ::testing::Test {
protected:
    DoFCameraTest() 
        : camera()
        , imageW(320)
        , imageH(240)
        , center(0.0, 0.0, 100.0)
        , direction(0.0, 0.0, -1.0)
        , up(0.0, 1.0, 0.0)
        , sensorSize(50.0)
        , distSensorToLens(20.0)
        , focalLength(80.0)
        , lensRadius(5.0)
        , sensorSensitivity(20.0) {
    }

    virtual ~DoFCameraTest() {
    }

    virtual void SetUp() {
        camera = DoFCamera(imageW, imageH,
                           center,
                           direction,
                           up,
                           sensorSize,
                           distSensorToLens,
                           focalLength,
                           lensRadius,
                           sensorSensitivity);
    }

    virtual void TearDown() {
    }

protected:
    DoFCamera camera;

    int imageW;
    int imageH;
    Point center;
    Vector3D direction;
    Vector3D up;
    double sensorSize;
    double distSensorToLens;
    double focalLength;
    double lensRadius;
    double sensorSensitivity;
};

TEST_F(DoFCameraTest, DefaultConstructor) {
    DoFCamera c;
    EXPECT_EQ(0.0, c.imageW());
    EXPECT_EQ(0.0, c.imageH());
}

TEST_F(DoFCameraTest, InstanceTest) {
    EXPECT_EQ(imageW, camera.imageW());
    EXPECT_EQ(imageH, camera.imageH());
    EXPECT_EQ_VEC(center, camera.center());
    EXPECT_EQ_VEC(direction, camera.direction());
    EXPECT_EQ(distSensorToLens, camera.distSL());
    EXPECT_EQ(focalLength, camera.focalLength());

    DoFCamera cp(camera);
    EXPECT_EQ(imageW, cp.imageW());
    EXPECT_EQ(imageH, cp.imageH());
    EXPECT_EQ_VEC(center, cp.center());
    EXPECT_EQ_VEC(direction, cp.direction());
    EXPECT_EQ(distSensorToLens, cp.distSL());
    EXPECT_EQ(focalLength, cp.focalLength());
}

TEST_F(DoFCameraTest, LensIntersection) {
    // Intersect
    Point posOnLens, posOnObjplane, posOnSensor;
    Vector3D uvOnSensor;
    Ray ray(Point(0.0, 0.0, 0.0), Vector3D(0.0, 0.0, 1.0));
    EXPECT_EQ(80.0, camera.intersectLens(ray, &posOnLens, &posOnObjplane, &posOnSensor, &uvOnSensor));
    EXPECT_EQ_VEC(Point(0.0, 0.0, 80.0), posOnLens);
    EXPECT_EQ_VEC(Point(0.0, 0.0, 100.0), posOnSensor);
    EXPECT_EQ_VEC(Point(0.0, 0.0, 0.0), posOnObjplane);
    EXPECT_EQ_VEC(Vector3D(imageW / 2.0, imageH / 2.0, 0.0), uvOnSensor);

    // Not intersect
    ray = Ray(Point(0.0, 0.0, 0.0), Vector3D(0.0, 1.0, 0.0));
    EXPECT_EQ(-INFTY, camera.intersectLens(ray, &posOnLens, &posOnObjplane, &posOnSensor, &uvOnSensor));
}

TEST_F(DoFCameraTest, SampleTest) {
    const int numSample = 100;
    Random rng = Random();

    Point posOnLens, posOnObjplane, posOnSensor;
    Vector3D uvOnSensor;
    double pImage, pLens;
    for (int i = 0; i < numSample; i++) {
        camera.samplePoints(imageW / 2, imageH / 2, rng, &posOnSensor, &posOnObjplane, &posOnLens, &pImage, &pLens);
        EXPECT_EQ(80.0, posOnLens.z());
        EXPECT_EQ(100.0, posOnSensor.z());
        EXPECT_EQ(1.0 / camera.lensArea(), pLens);
    }
}

// -----------------------------------------------------------------------------
// Perspetcive Camera Test
// -----------------------------------------------------------------------------

class PerspectiveCameraTest : public ::testing::Test {
protected:
    PerspectiveCameraTest() {}
    virtual ~PerspectiveCameraTest() {}
};

TEST_F(PerspectiveCameraTest, DefaultInstance) {
    PerspectiveCamera camera;
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 0.0), camera.center());
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 0.0), camera.direction());
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 0.0), camera.up());
    EXPECT_EQ(0.0, camera.aspect());
    EXPECT_EQ(0.0, camera.fov());
    EXPECT_EQ(0, camera.imageW());
    EXPECT_EQ(0, camera.imageH());
}

TEST_F(PerspectiveCameraTest, Instance) {
    Point    center(0.0, 0.0, 0.0);
    Vector3D direction(0.0, 0.0, 1.0);
    Vector3D up(0.0, 1.0, 0.0);
    const int width = 320;
    const int height = 240;

    Camera camera = Camera::perspective(center, direction, up, 45.0, width, height, 1.0);
    EXPECT_EQ_VEC(center, camera.center());
    EXPECT_EQ_VEC(direction, camera.direction());
    EXPECT_EQ_VEC(up, camera.up());
    EXPECT_EQ(width, camera.imageW());
    EXPECT_EQ(height, camera.imageH());
    EXPECT_EQ(1.0, camera.sensitivity());
}

// -----------------------------------------------------------------------------
// Orthogonal Camera Test
// -----------------------------------------------------------------------------

class OrthographicCameraTest : public ::testing::Test {
protected:
    OrthographicCameraTest() {}
    virtual ~OrthographicCameraTest() {}
};

TEST_F(OrthographicCameraTest, DefaultInstance) {
    OrthographicCamera camera;
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 0.0), camera.center());
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 0.0), camera.direction());
    EXPECT_EQ_VEC(Vector3D(0.0, 0.0, 0.0), camera.up());
    EXPECT_EQ(0, camera.imageW());
    EXPECT_EQ(0, camera.imageH());
    EXPECT_EQ(0.0, camera.sensitivity());
}

TEST_F(OrthographicCameraTest, Instance) {
    Point    center(0.0, 0.0, 0.0);
    Vector3D direction(0.0, 0.0, 1.0);
    Vector3D up(0.0, 1.0, 0.0);
    const int width = 320;
    const int height = 240;
    Rect rect(0, 0, width, height);

    Camera camera = Camera::ortho(center, direction, up, rect, width, height, 1.0);
    EXPECT_EQ_VEC(center, camera.center());
    EXPECT_EQ_VEC(direction, camera.direction());
    EXPECT_EQ_VEC(up, camera.up());
    EXPECT_EQ(width, camera.imageW());
    EXPECT_EQ(height, camera.imageH());
    EXPECT_EQ(1.0, camera.sensitivity());
}
