#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

class AreaLightTest : public ::testing::Test {
protected:
    AreaLightTest() {}
    virtual ~AreaLightTest() {}
};

TEST_F(AreaLightTest, Instance) {
    const Vector3D v00(-10.0, 0.0, -10.0);
    const Vector3D v01(-10.0, 0.0,  10.0);
    const Vector3D v10( 10.0, 0.0, -10.0);
    const Vector3D v11( 10.0, 0.0,  10.0);
    Quad quad(v00, v01, v11, v10);
    Lighting light = Lighting::asAreaLight(quad.triangulate(), Color(32.0, 32.0, 32.0));

    EXPECT_EQ(quad.area(), light.area());
}

class EnvmapTest : public ::testing::Test {
protected:
    EnvmapTest() {}
    virtual ~EnvmapTest() {}
};

TEST_F(EnvmapTest, Instance) {
    Sphere boundSphere(Vector3D(0.0, 0.0, 0.0), 10.0);
    Image image = Image::fromFile(kDataDirectory + "cave_room.hdr");
    Lighting light = Lighting::asEnvmap(boundSphere, image);
    
    EXPECT_EQ(boundSphere.area(), light.area());
}