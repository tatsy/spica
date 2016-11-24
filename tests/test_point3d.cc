#include "gtest/gtest.h"

#include <ctime>

#include "../include/spica.h"
using namespace spica;

class Point3dTest : public ::testing::Test {
protected:
    Point3dTest() {}
    virtual ~Point3dTest() {}
};

TEST_F(Point3dTest, DefaultInstance) {
    Point3d p;
    EXPECT_EQ(0.0, p.x());
    EXPECT_EQ(0.0, p.y());
    EXPECT_EQ(0.0, p.z());
}

TEST_F(Point3dTest, InstanceWithParameters) {
    Random rnd((unsigned int)time(0));
    double rx = rnd.nextReal();
    double ry = rnd.nextReal();
    double rz = rnd.nextReal();

    Point3d p(rx, ry, rz);
    EXPECT_EQ(rx, p.x());
    EXPECT_EQ(ry, p.y());
    EXPECT_EQ(rz, p.z());
}

TEST_F(Point3dTest, CopyAndAssign) {
    Random rnd((unsigned int)time(0));
    double rx = rnd.nextReal();
    double ry = rnd.nextReal();
    double rz = rnd.nextReal();
    Point3d p0(rx, ry, rz);

    Point3d p1(p0);
    EXPECT_EQ(rx, p1.x());
    EXPECT_EQ(ry, p1.y());
    EXPECT_EQ(rz, p1.z());

    Point3d p2;
    p2 = p0;
    EXPECT_EQ(rx, p2.x());
    EXPECT_EQ(ry, p2.y());
    EXPECT_EQ(rz, p2.z());
}

TEST_F(Point3dTest, InstanceWithVector3d) {
    Random rnd((unsigned int)time(0));
    double rx = rnd.nextReal();
    double ry = rnd.nextReal();
    double rz = rnd.nextReal();

    Vector3d v(rx, ry, rz);
    Point3d p(v);
    EXPECT_EQ(rx, p.x());
    EXPECT_EQ(ry, p.y());
    EXPECT_EQ(rz, p.z());
}

TEST_F(Point3dTest, MaximumAndMinimum) {
    Random rnd((unsigned int)time(0));
    Point3d p1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    Point3d p2(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    Point3d p3 = Point3d::maximum(p1, p2);
    EXPECT_EQ(std::max(p1.x(), p2.x()), p3.x());
    EXPECT_EQ(std::max(p1.y(), p2.y()), p3.y());
    EXPECT_EQ(std::max(p1.z(), p2.z()), p3.z());

    Point3d p4 = Point3d::minimum(p1, p2);
    EXPECT_EQ(std::min(p1.x(), p2.x()), p4.x());
    EXPECT_EQ(std::min(p1.y(), p2.y()), p4.y());
    EXPECT_EQ(std::min(p1.z(), p2.z()), p4.z());
}

TEST_F(Point3dTest, AddAndSubtract) {
    Random rnd((unsigned int)time(0));
    Point3d  p1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    Point3d  p2(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    Vector3d v1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    Point3d p3 = p1 + p2;
    EXPECT_DOUBLE_EQ(p1.x() + p2.x(), p3.x());
    EXPECT_DOUBLE_EQ(p1.y() + p2.y(), p3.y());
    EXPECT_DOUBLE_EQ(p1.z() + p2.z(), p3.z());

    Vector3d v2 = p1 - p2;
    EXPECT_DOUBLE_EQ(p1.x() - p2.x(), v2.x());
    EXPECT_DOUBLE_EQ(p1.y() - p2.y(), v2.y());
    EXPECT_DOUBLE_EQ(p1.z() - p2.z(), v2.z());

    Point3d p4 = p1 - v1;
    EXPECT_DOUBLE_EQ(p1.x() - v1.x(), p4.x());
    EXPECT_DOUBLE_EQ(p1.y() - v1.y(), p4.y());
    EXPECT_DOUBLE_EQ(p1.z() - v1.z(), p4.z());
}

TEST_F(Point3dTest, MultiplyAndDivision) {
    Random rnd((unsigned int)time(0));
    Point3d  p1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    double   s = rnd.nextReal();
    while (s == 0.0) s = rnd.nextReal();

    Point3d p2 = p1 * s;
    EXPECT_DOUBLE_EQ(p1.x() * s, p2.x());
    EXPECT_DOUBLE_EQ(p1.y() * s, p2.y());
    EXPECT_DOUBLE_EQ(p1.z() * s, p2.z());

    Point3d p3 = s * p1;
    EXPECT_DOUBLE_EQ(p1.x() * s, p3.x());
    EXPECT_DOUBLE_EQ(p1.y() * s, p3.y());
    EXPECT_DOUBLE_EQ(p1.z() * s, p3.z());

    Point3d p4 = p1 / s;
    EXPECT_DOUBLE_EQ(p1.x() / s, p4.x());
    EXPECT_DOUBLE_EQ(p1.y() / s, p4.y());
    EXPECT_DOUBLE_EQ(p1.z() / s, p4.z());

    double t = 0.0;
    ASSERT_DEATH(p1 / t, "");
}