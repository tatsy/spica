#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

// ------------------------------
// Camera class test
// ------------------------------
class CameraTest : public ::testing::Test {
protected:
    CameraTest() 
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
        , sensorSensitivity(20.0)
    {
    }

    virtual ~CameraTest() {
    }

    virtual void SetUp() {
        camera = Camera(imageW, imageH,
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
    Camera camera;

    int imageW;
    int imageH;
    Vector3 center;
    Vector3 direction;
    Vector3 up;
    double sensorSize;
    double distSensorToLens;
    double focalLength;
    double lensRadius;
    double sensorSensitivity;
};



TEST_F(CameraTest, DefaultConstructor) {
    Camera c;
    EXPECT_EQ(0.0, c.imageW());
    EXPECT_EQ(0.0, c.imageH());
}

TEST_F(CameraTest, InstanceTest) {
    EXPECT_EQ(imageW, camera.imageW());
    EXPECT_EQ(imageH, camera.imageH());
    EXPECT_EQ_VEC(center, camera.center());
    EXPECT_EQ_VEC(direction, camera.direction());
    EXPECT_EQ(distSensorToLens, camera.distSL());
    EXPECT_EQ(focalLength, camera.focalLength());

    Camera cp(camera);
    EXPECT_EQ(imageW, cp.imageW());
    EXPECT_EQ(imageH, cp.imageH());
    EXPECT_EQ_VEC(center, cp.center());
    EXPECT_EQ_VEC(direction, cp.direction());
    EXPECT_EQ(distSensorToLens, cp.distSL());
    EXPECT_EQ(focalLength, cp.focalLength());
}

TEST_F(CameraTest, LensIntersection) {
    // Intersect
    Vector3 posOnLens, posOnObjplane, posOnSensor, uvOnSensor;
    Ray ray(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0));
    EXPECT_EQ(80.0, camera.intersectLens(ray, posOnLens, posOnObjplane, posOnSensor, uvOnSensor));
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 80.0), posOnLens);
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 100.0), posOnSensor);
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 0.0), posOnObjplane);
    EXPECT_EQ_VEC(Vector3(imageW / 2.0, imageH / 2.0, 0.0), uvOnSensor);

    // Not intersect
    ray = Ray(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));
    EXPECT_EQ(-INFTY, camera.intersectLens(ray, posOnLens, posOnObjplane, posOnSensor, uvOnSensor));
}

TEST_F(CameraTest, SampleTest) {
    const int numSample = 100;
    Random rng = Random();

    Vector3 posOnLens, posOnObjplane, posOnSensor, uvOnSensor;
    double pImage, pLens;
    for (int i = 0; i < numSample; i++) {
        camera.samplePoints(imageW / 2, imageH / 2, rng, posOnSensor, posOnObjplane, posOnLens, pImage, pLens);
        EXPECT_EQ(80.0, posOnLens.z());
        EXPECT_EQ(100.0, posOnSensor.z());
        EXPECT_EQ(1.0 / camera.lensArea(), pLens);
    }
}

