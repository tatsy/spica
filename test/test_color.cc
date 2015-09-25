#include "gtest/gtest.h"

#include <tuple>

#include "../include/spica.h"
using namespace spica;

// -----------------------------------------------------------------------------
// Color class test
// -----------------------------------------------------------------------------

class ColorTest : public ::testing::Test {
protected:
    ColorTest() {}
    virtual ~ColorTest() {}
}; 

class ColorTestWithParam : public ColorTest, public ::testing::WithParamInterface<std::tuple<double, double, double> > {
protected:
    double d0, d1, d2;

protected:
    ColorTestWithParam() {
        auto t = GetParam();
        d0 = std::get<0>(t);
        d1 = std::get<1>(t);
        d2 = std::get<2>(t);
    }

    virtual ~ColorTestWithParam() {
    }
};

TEST_P(ColorTestWithParam, DefaultInstance) {
    Color c0;
    EXPECT_EQ(0.0, c0.red());
    EXPECT_EQ(0.0, c0.green());
    EXPECT_EQ(0.0, c0.blue());
}

TEST_P(ColorTestWithParam, InstanceWithParams) {
    Color c1(d0, d1, d2);
    EXPECT_EQ(d0, c1.red());
    EXPECT_EQ(d1, c1.green());
    EXPECT_EQ(d2, c1.blue());
}

TEST_P(ColorTestWithParam, CopyConstructor) {
    Color c0(d0, d1, d2);
    Color c1(c0);
    EXPECT_EQ(d0, c1.red());
    EXPECT_EQ(d1, c1.green());
    EXPECT_EQ(d2, c1.blue());
}

TEST_P(ColorTestWithParam, Assignment) {
    Color c;

    c = Color(d0, d1, d2);
    EXPECT_EQ(d0, c.red());
    EXPECT_EQ(d1, c.green());
    EXPECT_EQ(d2, c.blue());

    double lum = c.red() * 0.2126 + c.green() * 0.7152 + c.blue() * 0.0722;
    EXPECT_DOUBLE_EQ(lum, c.luminance());
}

TEST_P(ColorTestWithParam, ValidSqrt) {
    Color u(d0, d1, d2);
    Color v = Color::sqrt(u);
    EXPECT_EQ(sqrt(u.red()), v.red());
    EXPECT_EQ(sqrt(u.green()), v.green());
    EXPECT_EQ(sqrt(u.blue()), v.blue());
}

TEST_F(ColorTest, InvalidSqrt) {
    Color c(-1.0, 2.0, 3.0);
    ASSERT_DEATH(Color::sqrt(c), "");
}

TEST_P(ColorTestWithParam, ExpTest) {
    Color u(d0, d1, d2);
    Color v = Color::exp(u);
    EXPECT_EQ(exp(u.red()), v.red());
    EXPECT_EQ(exp(u.green()), v.green());
    EXPECT_EQ(exp(u.blue()), v.blue());
}

const std::vector<double> values = { 0.0, 0.25, 0.50, 0.75 };
INSTANTIATE_TEST_CASE_P(, ColorTestWithParam,
    ::testing::Combine(::testing::ValuesIn(values),
                       ::testing::ValuesIn(values),
                       ::testing::ValuesIn(values)));
