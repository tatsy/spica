#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

// -----------------------------------------------------------------------------
// Color class test
// -----------------------------------------------------------------------------

class ColorTest : public ::testing::TestWithParam<double> {
protected:
    ColorTest() {
    }

    ~ColorTest() {
    }
};

TEST_F(ColorTest, InstanceTest) {
    Color c0;
    EXPECT_EQ(0.0, c0.red());
    EXPECT_EQ(0.0, c0.green());
    EXPECT_EQ(0.0, c0.blue());

    Color c1(0.5, 0.75, 0.25);
    EXPECT_EQ(0.5, c1.red());
    EXPECT_EQ(0.75, c1.green());
    EXPECT_EQ(0.25, c1.blue());
}

TEST_F(ColorTest, CopyConstructor) {
    Color c0(0.5, 0.75, 0.25);
    Color c1(c0);
    EXPECT_EQ(0.5, c1.red());
    EXPECT_EQ(0.75, c1.green());
    EXPECT_EQ(0.25, c1.blue());
}

TEST_F(ColorTest, OtherTest) {
    Color c(0.0, 0.1, 0.2);
    EXPECT_EQ(0.0, c.red());
    EXPECT_EQ(0.1, c.green());
    EXPECT_EQ(0.2, c.blue());

    c = Color(0.5, 0.6, 0.7);
    EXPECT_EQ(0.5, c.red());
    EXPECT_EQ(0.6, c.green());
    EXPECT_EQ(0.7, c.blue());

    double lum = c.red() * 0.2126 + c.green() * 0.7152 + c.blue() * 0.0722;
    EXPECT_DOUBLE_EQ(lum, c.luminance());
}

TEST_F(ColorTest, SqrtTest) {
    Color u(1.0, 2.0, 3.0);
    Color v = Color::sqrt(u);
    EXPECT_EQ(sqrt(u.red()), v.red());
    EXPECT_EQ(sqrt(u.green()), v.green());
    EXPECT_EQ(sqrt(u.blue()), v.blue());

    u = Color(-1.0, 2.0, 3.0);
    ASSERT_DEATH(Color::sqrt(u), "");
}

TEST_F(ColorTest, ExpTest) {
    Color u(1.0, 2.0, 3.0);
    Color v = Color::exp(u);
    EXPECT_EQ(exp(u.red()), v.red());
    EXPECT_EQ(exp(u.green()), v.green());
    EXPECT_EQ(exp(u.blue()), v.blue());
}
