#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

// ------------------------------
// Color class test
// ------------------------------
TEST(ColorTest, InstanceTest) {
    Color c0;
    EXPECT_EQ(0.0, c0.red());
    EXPECT_EQ(0.0, c0.green());
    EXPECT_EQ(0.0, c0.blue());

    Color c1(0.5, 0.75, 0.25);
    EXPECT_EQ(0.5, c1.red());
    EXPECT_EQ(0.75, c1.green());
    EXPECT_EQ(0.25, c1.blue());
}

TEST(ColorTest, CopyConstructor) {
    Color c0(0.5, 0.75, 0.25);
    Color c1(c0);
    EXPECT_EQ(0.5, c1.red());
    EXPECT_EQ(0.75, c1.green());
    EXPECT_EQ(0.25, c1.blue());
}

TEST(ColorTest, OtherTest) {
    Color c(0.0, 0.1, 0.2);
    EXPECT_EQ(0.0, c.red());
    EXPECT_EQ(0.1, c.green());
    EXPECT_EQ(0.2, c.blue());

    c = Vector3D(0.5, 0.6, 0.7);
    EXPECT_EQ(0.5, c.red());
    EXPECT_EQ(0.6, c.green());
    EXPECT_EQ(0.7, c.blue());

    double lum = c.red() * 0.2126 + c.green() * 0.7152 + c.blue() * 0.0722;
    EXPECT_DOUBLE_EQ(lum, c.luminance());
}

// ------------------------------
// Material class test
// ------------------------------
TEST(MaterialTest, InstanceTest) {
    Material mtrl;

    EXPECT_EQ(0.0, mtrl.emission.red());
    EXPECT_EQ(0.0, mtrl.emission.green());
    EXPECT_EQ(0.0, mtrl.emission.blue());

    EXPECT_EQ(0.0, mtrl.color.red());
    EXPECT_EQ(0.0, mtrl.color.green());
    EXPECT_EQ(0.0, mtrl.color.blue());

    EXPECT_EQ(REFLECTION_DIFFUSE, mtrl.reftype);

    mtrl = Material(Color(16.0, 15.0, 14.0),
                    Color(0.25, 0.50, 0.75),
                    REFLECTION_SPECULAR);

    EXPECT_EQ(16.0, mtrl.emission.red());
    EXPECT_EQ(15.0, mtrl.emission.green());
    EXPECT_EQ(14.0, mtrl.emission.blue());

    EXPECT_EQ(0.25, mtrl.color.red());
    EXPECT_EQ(0.50, mtrl.color.green());
    EXPECT_EQ(0.75, mtrl.color.blue());

    EXPECT_EQ(REFLECTION_SPECULAR, mtrl.reftype);
}