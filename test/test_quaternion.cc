#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include <vector>

class QuaternionTest : public ::testing::Test {
protected:
    QuaternionTest() {}
    virtual ~QuaternionTest() {}
};

class QuaternionUnaryTest : public QuaternionTest, 
                            public ::testing::WithParamInterface<Quaternion> {
protected:
    Quaternion q1;

protected:
    QuaternionUnaryTest() {}
    virtual ~QuaternionUnaryTest() {}

    void SetUp() {
        q1 = GetParam();
    }
};

TEST_F(QuaternionTest, DefaultInstance) {
    Quaternion q;
    EXPECT_EQ(0.0, q.x());
    EXPECT_EQ(0.0, q.y());
    EXPECT_EQ(0.0, q.z());
    EXPECT_EQ(0.0, q.w());
}

TEST_P(QuaternionUnaryTest, Instance) {
    Quaternion q(q1.x(), q1.y(), q1.z(), q1.w());
    EXPECT_EQ(q1.x(), q.x());
    EXPECT_EQ(q1.y(), q.y());
    EXPECT_EQ(q1.z(), q.z());
    EXPECT_EQ(q1.w(), q.w());
}

TEST_P(QuaternionUnaryTest, InstanceWithVector) {
    Vector3D v(q1.x(), q1.y(), q1.z());
    Quaternion q(v);
    EXPECT_EQ(v.x(), q.x());
    EXPECT_EQ(v.y(), q.y());
    EXPECT_EQ(v.z(), q.z());
    EXPECT_EQ(0.0,   q.w());
}

TEST_P(QuaternionUnaryTest, CopyConstruct) {
    Quaternion q(q1);
    EXPECT_EQ(q1.x(), q.x());
    EXPECT_EQ(q1.y(), q.y());
    EXPECT_EQ(q1.z(), q.z());
    EXPECT_EQ(q1.w(), q.w());
}

TEST_P(QuaternionUnaryTest, Assignment) {
    Quaternion q;
    q = q1;
    EXPECT_EQ(q1.x(), q.x());
    EXPECT_EQ(q1.y(), q.y());
    EXPECT_EQ(q1.z(), q.z());
    EXPECT_EQ(q1.w(), q.w());
}

TEST_P(QuaternionUnaryTest, Norm) {
    double sqnrm = q1.x() * q1.x() +
                   q1.y() * q1.y() +
                   q1.z() * q1.z() +
                   q1.w() * q1.w();
    EXPECT_EQ(sqnrm, q1.squaredNorm());
    EXPECT_EQ(sqrt(sqnrm), q1.norm());
}

TEST_F(QuaternionTest, SimpleRotation) {
    Vector3D v(1.0, 0.0, 0.0);
    Vector3D axis(0.0, 1.0, 0.0);
    Quaternion r    = Quaternion::rotation(axis,  PI * 0.5);
    std::cout << r << std::endl;
    Vector3D u = (r * Quaternion(v) * r.inverse()).toVector3D();

    EXPECT_DOUBLE_EQ( 0.0, u.x());
    EXPECT_DOUBLE_EQ( 0.0, u.y());
    EXPECT_DOUBLE_EQ(-1.0, u.z());
}

std::vector<Quaternion> quats = {
    Quaternion(0.0, 0.0, 0.0, 0.0),
    Quaternion(1.0, 2.0, 3.0, 4.0),
    Quaternion(-1.0, -2.0, -3.0, -4.0),
    Quaternion(1.0e8, 1.0e8, 1.0e8, 1.0e8),
    Quaternion(1.0e-8, 1.0e-8, 1.0e-8, 1.0e-8),
};

INSTANTIATE_TEST_CASE_P(, QuaternionUnaryTest,
                          ::testing::ValuesIn(quats));