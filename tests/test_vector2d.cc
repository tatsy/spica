#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

template <class T>
class Vector2dTest : public ::testing::Test {
protected:
    Vector2dTest() {}
    virtual ~Vector2dTest() {}
};

TYPED_TEST_CASE_P(Vector2dTest);

TYPED_TEST_P(Vector2dTest, DefaultInstance) {
    Vector2_<TypeParam> v;
    EXPECT_EQ(0, v.x());
    EXPECT_EQ(0, v.y());
}

TYPED_TEST_P(Vector2dTest, Instance) {
    const int trials = 100;
    Random rng((unsigned int)time(0));

    for (int t = 0; t < trials; t++) {
        auto x = static_cast<TypeParam>(rng.nextReal());
        auto y = static_cast<TypeParam>(rng.nextReal());
        Vector2_<TypeParam> v(x, y);
        EXPECT_EQ(x, v.x());
        EXPECT_EQ(y, v.y());
    }
}

TYPED_TEST_P(Vector2dTest, CopyAndAssign) {
    const int trials = 100;
    Random rng((unsigned int)time(0));

    for (int t = 0; t < trials; t++) {
        auto x = static_cast<TypeParam>(rng.nextReal());
        auto y = static_cast<TypeParam>(rng.nextReal());
        Vector2_<TypeParam> v(x, y);

        Vector2_<TypeParam> u = v;
        EXPECT_EQ(v.x(), u.x());
        EXPECT_EQ(v.y(), u.y());

        Vector2_<TypeParam> w;
        w = v;
        EXPECT_EQ(v.x(), w.x());
        EXPECT_EQ(v.y(), w.y());
    }
}

REGISTER_TYPED_TEST_CASE_P(Vector2dTest,
                           DefaultInstance,
                           Instance,
                           CopyAndAssign);

using VectorTypes = ::testing::Types<int, float, double>;
INSTANTIATE_TYPED_TEST_CASE_P(, Vector2dTest, VectorTypes);
