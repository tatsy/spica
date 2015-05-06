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