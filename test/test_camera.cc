#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

// ------------------------------
// Camera class test
// ------------------------------
TEST(CameraTest, DefaultConstructor) {
    Camera camera;
    EXPECT_EQ(0.0, camera.imageW());
    EXPECT_EQ(0.0, camera.imageH());
}

TEST(CameraTest, InstanceTest) {
    Camera camera(320, 240,
                  Vector3(0.0, 0.0, 100.0),
                  Vector3(0.0, 0.0, -1.0),
                  Vector3(0.0, 1.0, 0.0),
                  50.0,
                  20.0,
                  80.0,
                  5.0,
                  20.0);
    EXPECT_EQ(320.0, camera.imageW());
    EXPECT_EQ(240.0, camera.imageH());
}
