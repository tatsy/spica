#include "gtest/gtest.h"

#include <ctime>

#include "../include/spica.h"
using namespace spica;

class Point3DTest : public ::testing::Test {
protected:
    Point3DTest() {}
    virtual ~Point3DTest() {}
};

TEST_F(Point3DTest, DefaultInstance) {
    Point3D p;
    EXPECT_EQ(0.0, p.x());
    EXPECT_EQ(0.0, p.y());
    EXPECT_EQ(0.0, p.z());
}

TEST_F(Point3DTest, InstanceWithParameters) {
    Random rnd((unsigned int)time(0));
    double rx = rnd.nextReal();
    double ry = rnd.nextReal();
    double rz = rnd.nextReal();

    Point3D p(rx, ry, rz);
    EXPECT_EQ(rx, p.x());
    EXPECT_EQ(ry, p.y());
    EXPECT_EQ(rz, p.z());
}

TEST_F(Point3DTest, CopyAndAssign) {
    Random rnd((unsigned int)time(0));
    double rx = rnd.nextReal();
    double ry = rnd.nextReal();
    double rz = rnd.nextReal();
    Point3D p0(rx, ry, rz);

    Point3D p1(p0);
    EXPECT_EQ(rx, p1.x());
    EXPECT_EQ(ry, p1.y());
    EXPECT_EQ(rz, p1.z());

    Point3D p2;
    p2 = p0;
    EXPECT_EQ(rx, p2.x());
    EXPECT_EQ(ry, p2.y());
    EXPECT_EQ(rz, p2.z());
}

TEST_F(Point3DTest, InstanceWithVector3D) {
    Random rnd((unsigned int)time(0));
    double rx = rnd.nextReal();
    double ry = rnd.nextReal();
    double rz = rnd.nextReal();

    Vector3D v(rx, ry, rz);
    Point3D p(v);
    EXPECT_EQ(rx, p.x());
    EXPECT_EQ(ry, p.y());
    EXPECT_EQ(rz, p.z());
}

TEST_F(Point3DTest, MaximumAndMinimum) {
    Random rnd((unsigned int)time(0));
    Point3D p1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    Point3D p2(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    Point3D p3 = Point3D::maximum(p1, p2);
    EXPECT_EQ(std::max(p1.x(), p2.x()), p3.x());
    EXPECT_EQ(std::max(p1.y(), p2.y()), p3.y());
    EXPECT_EQ(std::max(p1.z(), p2.z()), p3.z());

    Point3D p4 = Point3D::minimum(p1, p2);
    EXPECT_EQ(std::min(p1.x(), p2.x()), p4.x());
    EXPECT_EQ(std::min(p1.y(), p2.y()), p4.y());
    EXPECT_EQ(std::min(p1.z(), p2.z()), p4.z());
}

TEST_F(Point3DTest, AddAndSubtract) {
    Random rnd((unsigned int)time(0));
    Point3D  p1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    Point3D  p2(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    Vector3D v1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    Point3D p3 = p1 + p2;
    EXPECT_DOUBLE_EQ(p1.x() + p2.x(), p3.x());
    EXPECT_DOUBLE_EQ(p1.y() + p2.y(), p3.y());
    EXPECT_DOUBLE_EQ(p1.z() + p2.z(), p3.z());

    Vector3D v2 = p1 - p2;
    EXPECT_DOUBLE_EQ(p1.x() - p2.x(), v2.x());
    EXPECT_DOUBLE_EQ(p1.y() - p2.y(), v2.y());
    EXPECT_DOUBLE_EQ(p1.z() - p2.z(), v2.z());

    Point3D p4 = p1 - v1;
    EXPECT_DOUBLE_EQ(p1.x() - v1.x(), p4.x());
    EXPECT_DOUBLE_EQ(p1.y() - v1.y(), p4.y());
    EXPECT_DOUBLE_EQ(p1.z() - v1.z(), p4.z());
}

TEST_F(Point3DTest, MultiplyAndDivision) {
    Random rnd((unsigned int)time(0));
    Point3D  p1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    double   s = rnd.nextReal();
    while (s == 0.0) s = rnd.nextReal();

    Point3D p2 = p1 * s;
    EXPECT_DOUBLE_EQ(p1.x() * s, p2.x());
    EXPECT_DOUBLE_EQ(p1.y() * s, p2.y());
    EXPECT_DOUBLE_EQ(p1.z() * s, p2.z());

    Point3D p3 = s * p1;
    EXPECT_DOUBLE_EQ(p1.x() * s, p3.x());
    EXPECT_DOUBLE_EQ(p1.y() * s, p3.y());
    EXPECT_DOUBLE_EQ(p1.z() * s, p3.z());

    Point3D p4 = p1 / s;
    EXPECT_DOUBLE_EQ(p1.x() / s, p4.x());
    EXPECT_DOUBLE_EQ(p1.y() / s, p4.y());
    EXPECT_DOUBLE_EQ(p1.z() / s, p4.z());

    double t = 0.0;
    ASSERT_DEATH(p1 / t, "");
}