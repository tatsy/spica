#include "gtest/gtest.h"

#include <cstdio>
#include <sstream>
#include <tuple>
#include <algorithm>

#include "../include/spica.h"
using namespace spica;

using Vector3DPair = std::tuple<Vector3D, Vector3D>; 

class Vector3DTest : public ::testing::Test {
protected:
    Vector3DTest() {}
    virtual ~Vector3DTest() {}
};

class Vector3DUnaryTest : public Vector3DTest, public ::testing::WithParamInterface<Vector3D> {
protected:
    Vector3D v1;

protected:
    Vector3DUnaryTest() {}
    virtual ~Vector3DUnaryTest() {}

    void SetUp() {
        v1 = GetParam();
    }
};

class Vector3DPairwiseTest : public Vector3DTest, public ::testing::WithParamInterface<Vector3DPair> {
protected:
    Vector3D v1, v2;

protected:
    Vector3DPairwiseTest() {}
    virtual ~Vector3DPairwiseTest() {}

    void SetUp() {
        v1 = std::get<0>(GetParam());
        v2 = std::get<1>(GetParam());
    }
};

TEST_F(Vector3DTest, DefaultInstance) {
    Vector3D v;
    EXPECT_EQ(0.0, v.x());
    EXPECT_EQ(0.0, v.y());
    EXPECT_EQ(0.0, v.z());
}

TEST_P(Vector3DUnaryTest, Instance) {
    Vector3D u(v1.x(), v1.y(), v1.z());
    EXPECT_EQ(v1.x(), u.x());
    EXPECT_EQ(v1.y(), u.y());
    EXPECT_EQ(v1.z(), u.z());
}

TEST_P(Vector3DUnaryTest, Copy) {
    Vector3D v = Vector3D(v1);
    EXPECT_EQ(v1.x(), v.x());
    EXPECT_EQ(v1.y(), v.y());
    EXPECT_EQ(v1.z(), v.z());
}

TEST_P(Vector3DUnaryTest, Assignment) {
    Vector3D v;
    v = v1;
    EXPECT_EQ(v1.x(), v.x());
    EXPECT_EQ(v1.y(), v.y());
    EXPECT_EQ(v1.z(), v.z());
}

TEST_P(Vector3DPairwiseTest, PlusOperator) {
    Vector3D w = v1 + v2;
    EXPECT_EQ(v1.x() + v2.x(), w.x());
    EXPECT_EQ(v1.y() + v2.y(), w.y());
    EXPECT_EQ(v1.z() + v2.z(), w.z());

    const double a = v2.x();
    Vector3D v = v1 + a;
    EXPECT_EQ(v1.x() + a, v.x());
    EXPECT_EQ(v1.y() + a, v.y());
    EXPECT_EQ(v1.z() + a, v.z());

    const double b = v2.y();
    Vector3D u = b + v1;
    EXPECT_EQ(b + v1.x(), u.x());
    EXPECT_EQ(b + v1.y(), u.y());
    EXPECT_EQ(b + v1.z(), u.z());
}

TEST_P(Vector3DPairwiseTest, MinusOperator) {
    Vector3D w = v1 - v2;
    EXPECT_EQ(v1.x() - v2.x(), w.x());
    EXPECT_EQ(v1.y() - v2.y(), w.y());
    EXPECT_EQ(v1.z() - v2.z(), w.z());

    const double a = v2.x();
    Vector3D v = v1 - a;
    EXPECT_EQ(v1.x() - a, v.x());
    EXPECT_EQ(v1.y() - a, v.y());
    EXPECT_EQ(v1.z() - a, v.z());

    const double b = v2.y();
    Vector3D u = b - v1;
    EXPECT_EQ(b - v1.x(), u.x());
    EXPECT_EQ(b - v1.y(), u.y());
    EXPECT_EQ(b - v1.z(), u.z());
}

TEST_P(Vector3DPairwiseTest, Multiplication) {
    const Vector3D v = v1 * v2;
    EXPECT_EQ(v1.x() * v2.x(), v.x());
    EXPECT_EQ(v1.y() * v2.y(), v.y());
    EXPECT_EQ(v1.z() * v2.z(), v.z());

    const double a = v2.x();
    const Vector3D u = v1 * a;
    EXPECT_EQ(v1.x() * a, u.x());
    EXPECT_EQ(v1.y() * a, u.y());
    EXPECT_EQ(v1.z() * a, u.z());

    const double b = v2.y();
    const Vector3D w = b * v1;
    EXPECT_EQ(b * v1.x(), w.x());
    EXPECT_EQ(b * v1.y(), w.y());
    EXPECT_EQ(b * v1.z(), w.z());

    const Vector3D t = v1.multiply(v2);
    EXPECT_EQ(v1.x() * v2.x(), t.x());
    EXPECT_EQ(v1.y() * v2.y(), t.y());
    EXPECT_EQ(v1.z() * v2.z(), t.z());
}

TEST_P(Vector3DPairwiseTest, Division) {
    if (v2.x() != 0.0 && v2.y() != 0.0 && v2.z() != 0.0) {
        const Vector3D v = v1 / v2;
        EXPECT_EQ(v1.x() / v2.x(), v.x());
        EXPECT_EQ(v1.y() / v2.y(), v.y());
        EXPECT_EQ(v1.z() / v2.z(), v.z());
    } else {
        ASSERT_DEATH(v1 / v2, "");
    }

    const double a = v2.x();
    if (a != 0.0) {
        const Vector3D u = v1 / a;
        EXPECT_DOUBLE_EQ(v1.x() / a, u.x());
        EXPECT_DOUBLE_EQ(v1.y() / a, u.y());
        EXPECT_DOUBLE_EQ(v1.z() / a, u.z());
    } else {
        ASSERT_DEATH(v1 / a, "");
    }
}

TEST_P(Vector3DUnaryTest, NormAndNormalize) {
    const double sqnrm = v1.x() * v1.x() + 
                         v1.y() * v1.y() +
                         v1.z() * v1.z();
    EXPECT_EQ(sqnrm, v1.squaredNorm());

    const double nrm = sqrt(sqnrm);
    EXPECT_EQ(sqrt(sqnrm), v1.norm());

    if (nrm != 0.0) {
        const Vector3D w = v1.normalized();
        EXPECT_EQ(v1.x() / nrm, w.x());
        EXPECT_EQ(v1.y() / nrm, w.y());
        EXPECT_EQ(v1.z() / nrm, w.z());
        EXPECT_FLOAT_EQ(w.norm(), 1.0);
    } else {
        ASSERT_DEATH(v1.normalized(), "");
    }

    if (nrm != 0.0) {
        Vector3D v = Vector3D::normalize(v1);
        EXPECT_EQ(v1.x() / nrm, v.x());
        EXPECT_EQ(v1.y() / nrm, v.y());
        EXPECT_EQ(v1.z() / nrm, v.z());
        EXPECT_FLOAT_EQ(1.0, v.norm());
    } else {
        ASSERT_DEATH(Vector3D::normalize(v1), "");
    }
}

TEST_P(Vector3DUnaryTest, Negation) {
    Vector3D v = -v1;
    EXPECT_EQ(-v1.x(), v.x());
    EXPECT_EQ(-v1.y(), v.y());
    EXPECT_EQ(-v1.z(), v.z());
}

TEST_P(Vector3DPairwiseTest, Equal) {
    EXPECT_EQ(v1.x() == v2.x() && v1.y() == v2.y() && v1.z() == v2.z(), v1 == v2);
}

TEST_P(Vector3DPairwiseTest, DotAndCross) {
    double dt = v1.dot(v2);
    EXPECT_EQ(v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z(), dt);
    EXPECT_EQ(dt, v2.dot(v1));

    const Vector3D w = v1.cross(v2);
    EXPECT_EQ(v1.y() * v2.z() - v1.z() * v2.y(), w.x());
    EXPECT_EQ(v1.z() * v2.x() - v1.x() * v2.z(), w.y());
    EXPECT_EQ(v1.x() * v2.y() - v1.y() * v2.x(), w.z());
}

TEST_P(Vector3DPairwiseTest, MinimumAndMaximum) {
    const Vector3D v = Vector3D::minimum(v1, v2);
    EXPECT_EQ(std::min(v1.x(), v2.x()), v.x());
    EXPECT_EQ(std::min(v1.y(), v2.y()), v.y());
    EXPECT_EQ(std::min(v1.z(), v2.z()), v.z());

    const Vector3D u = Vector3D::maximum(v1, v2);
    EXPECT_EQ(std::max(v1.x(), v2.x()), u.x());
    EXPECT_EQ(std::max(v1.y(), v2.y()), u.y());
    EXPECT_EQ(std::max(v1.z(), v2.z()), u.z());
}

TEST_P(Vector3DUnaryTest, ToStringAndCout) {
    char str[1024];
    sprintf(str, "(%.8f, %.8f, %.8f)", v1.x(), v1.y(), v1.z());
    EXPECT_EQ(std::string(str), v1.toString());

    std::stringstream ss;
    ss << v1;
    EXPECT_EQ(ss.str(), v1.toString());
}

std::vector<Vector3D> vectors = {
    Vector3D(0.0, 1.0, 2.0),
    Vector3D(-2.0, -1.0, 0.0),
    Vector3D(3.14, 1.59, 2.65),
    Vector3D(1.0e8, 1.0e8, 1.0e8),
    Vector3D(1.0e-8, 1.0e-8, 1.0e-8)
};

INSTANTIATE_TEST_CASE_P(, Vector3DUnaryTest,
    ::testing::ValuesIn(vectors));

INSTANTIATE_TEST_CASE_P(, Vector3DPairwiseTest,
    ::testing::Combine(::testing::ValuesIn(vectors),
                       ::testing::ValuesIn(vectors)));