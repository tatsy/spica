#include "gtest/gtest.h"

#include <cstdio>
#include <sstream>
#include <tuple>

#include "../include/spica.h"
using namespace spica;

// -----------------------------------------------------------------------------
// Color class test
// -----------------------------------------------------------------------------

using ColorPair = std::tuple<Color, Color>;

//! Fixture class for non-parametric tests
class ColorTest : public ::testing::Test {
protected:
    ColorTest() {}
    virtual ~ColorTest() {}
}; 

//! Fixture class for value-parameterized tests
class ColorTestWithParam : public ColorTest, public ::testing::WithParamInterface<ColorPair> {
protected:
    Color c1, c2;

protected:
    ColorTestWithParam() {}
    virtual ~ColorTestWithParam() {}

    void SetUp() {
        c1 = std::get<0>(GetParam());
        c2 = std::get<1>(GetParam());
    }
};

TEST_F(ColorTest, DefaultInstance) {
    Color c0;
    EXPECT_EQ(0.0, c0.red());
    EXPECT_EQ(0.0, c0.green());
    EXPECT_EQ(0.0, c0.blue());
}

TEST_P(ColorTestWithParam, InstanceWithParams) {
    Color c(c1.red(), c1.green(), c1.blue());
    EXPECT_EQ(c1.red(), c.red());
    EXPECT_EQ(c1.green(), c.green());
    EXPECT_EQ(c1.blue(), c.blue());
}

TEST_P(ColorTestWithParam, CopyConstructor) {
    Color c(c1);
    EXPECT_EQ(c1.red(), c.red());
    EXPECT_EQ(c1.green(), c.green());
    EXPECT_EQ(c1.blue(), c.blue());
}

TEST_P(ColorTestWithParam, Assignment) {
    Color c;
    c = c1;
    EXPECT_EQ(c1.red(), c.red());
    EXPECT_EQ(c1.green(), c.green());
    EXPECT_EQ(c1.blue(), c.blue());

    const double lum = c.red()   * 0.2126 +
                       c.green() * 0.7152 +
                       c.blue()  * 0.0722;
    EXPECT_DOUBLE_EQ(lum, c.luminance());
}

TEST_P(ColorTestWithParam, PlusOperator) {
    Color c3 = c1 + c2;
    EXPECT_EQ(c1.red() + c2.red(), c3.red());
    EXPECT_EQ(c1.green() + c2.green(), c3.green());
    EXPECT_EQ(c1.blue() + c2.blue(), c3.blue());

    const double x = c2.red();
    Color c4 = c1 + x;
    EXPECT_EQ(c1.red() + x, c4.red());
    EXPECT_EQ(c1.green() + x, c4.green());
    EXPECT_EQ(c1.blue() + x, c4.blue());

    const double y = c2.green();
    Color c5 = y + c1;
    EXPECT_EQ(y + c1.red(), c5.red());
    EXPECT_EQ(y + c1.green(), c5.green());
    EXPECT_EQ(y + c1.blue(), c5.blue());
}

TEST_P(ColorTestWithParam, MinusOperator) {
    Color c3 = c1 - c2;
    EXPECT_EQ(c1.red() - c2.red(), c3.red());
    EXPECT_EQ(c1.green() - c2.green(), c3.green());
    EXPECT_EQ(c1.blue() - c2.blue(), c3.blue());

    const double x = c2.red();
    Color c4 = c1 - x;
    EXPECT_EQ(c1.red() - x, c4.red());
    EXPECT_EQ(c1.green() - x, c4.green());
    EXPECT_EQ(c1.blue() - x, c4.blue());

    const double y = c2.green();
    Color c5 = y - c1;
    EXPECT_EQ(y - c1.red(), c5.red());
    EXPECT_EQ(y - c1.green(), c5.green());
    EXPECT_EQ(y - c1.blue(), c5.blue());
}

TEST_P(ColorTestWithParam, Negation) {
    Color c = -c1;
    EXPECT_EQ(-c1.red(), c.red());
    EXPECT_EQ(-c1.green(), c.green());
    EXPECT_EQ(-c1.blue(), c.blue());
}

TEST_P(ColorTestWithParam, ScalarMultiplication) {
    const double d = c2.red();
    const Color c3 = c1 * d;
    EXPECT_EQ(c1.red()   * d, c3.red());
    EXPECT_EQ(c1.green() * d, c3.green());
    EXPECT_EQ(c1.blue()  * d, c3.blue());

    const double e = c2.green();
    const Color c4 = e * c1;
    EXPECT_EQ(e * c1.red(), c4.red());
    EXPECT_EQ(e * c1.green(), c4.green());
    EXPECT_EQ(e * c1.blue(), c4.blue());
}

TEST_P(ColorTestWithParam, ComponentWiseMultiplication) {
    Color c3 = c1 * c2;
    EXPECT_EQ(c1.red() * c2.red(), c3.red());
    EXPECT_EQ(c1.green() * c2.green(), c3.green());
    EXPECT_EQ(c1.blue() * c2.blue(), c3.blue());
}

TEST_P(ColorTestWithParam, Division) {
    const double d = c2.red();
    if (d != 0.0) {
        Color c = c1 / d;
        EXPECT_EQ(c1.red() / d, c.red());
        EXPECT_EQ(c1.green() / d, c.green());
        EXPECT_EQ(c1.blue() / d, c.blue());
    } else {
        ASSERT_DEATH(c1 / d, "");
    }
}

TEST_P(ColorTestWithParam, Norm) {
    const double sqnrm = c1.red() * c1.red() +
                         c1.green() * c1.green() + 
                         c1.blue() * c1.blue();
    EXPECT_EQ(sqnrm, c1.squaredNorm());
    EXPECT_EQ(sqrt(sqnrm), c1.norm());
}

TEST_P(ColorTestWithParam, MinimumAndMaximum) {
    const Color c3 = Color::minimum(c1, c2);
    EXPECT_EQ(std::min(c1.red(), c2.red()), c3.red());
    EXPECT_EQ(std::min(c1.green(), c2.green()), c3.green());
    EXPECT_EQ(std::min(c1.blue(), c2.blue()), c3.blue());

    const Color c4 = Color::maximum(c1, c2);
    EXPECT_EQ(std::max(c1.red(), c2.red()), c4.red());
    EXPECT_EQ(std::max(c1.green(), c2.green()), c4.green());
    EXPECT_EQ(std::max(c1.blue(), c2.blue()), c4.blue());
}

TEST_P(ColorTestWithParam, Clamp) {
    Color c3 = c1.clamp();
    EXPECT_EQ(std::max(0.0, std::min(c1.red(),   INFTY)), c3.red());
    EXPECT_EQ(std::max(0.0, std::min(c1.green(), INFTY)), c3.green());
    EXPECT_EQ(std::max(0.0, std::min(c1.blue(),  INFTY)), c3.blue());
}

TEST_P(ColorTestWithParam, ComponentWiseSqrt) {
    if (c1.red() >= 0.0 && c1.green() >= 0.0 && c1.blue() >= 0.0) {
        Color v = Color::sqrt(c1);
        EXPECT_EQ(sqrt(c1.red()), v.red());
        EXPECT_EQ(sqrt(c1.green()), v.green());
        EXPECT_EQ(sqrt(c1.blue()), v.blue());
    } else {
        ASSERT_DEATH(Color::sqrt(c1), "");
    }
}

TEST_P(ColorTestWithParam, ExpTest) {
    const Color v = Color::exp(c1);
    EXPECT_EQ(exp(c1.red()), v.red());
    EXPECT_EQ(exp(c1.green()), v.green());
    EXPECT_EQ(exp(c1.blue()), v.blue());
}

TEST_P(ColorTestWithParam, ToStringAndCout) {
    char str[1024];
    sprintf(str, "(%.8f, %.8f, %.8f)", c1.red(), c1.green(), c1.blue());
    EXPECT_EQ(std::string(str), c1.toString());

    std::stringstream ss;
    ss << c1;
    EXPECT_EQ(ss.str(), c1.toString());
}

std::vector<Color> colors = {
    Color( 0.0,    1.0,    2.0   ),
    Color(-2.0,   -1.0,    0.0   ),
    Color( 3.14,   1.59,   2.65  ),
    Color( 1.0e8,  1.0e8,  1.0e8 ),
    Color( 1.0e-8, 1.0e-8, 1.0e-8)
};

INSTANTIATE_TEST_CASE_P(, ColorTestWithParam,
    ::testing::Combine(::testing::ValuesIn(colors),
                       ::testing::ValuesIn(colors)));
                       