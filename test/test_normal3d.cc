#include "gtest/gtest.h"

#include <ctime>

#include "../include/spica.h"
using namespace spica;

class Normal3DTest : public ::testing::Test {
protected:
    Normal3DTest() {}
    virtual ~Normal3DTest() {}
};

TEST_F(Normal3DTest, DefaultInstance) {
    Normal3D n;
    EXPECT_EQ(0.0, n.x());
    EXPECT_EQ(0.0, n.y());
    EXPECT_EQ(0.0, n.z());
}

TEST_F(Normal3DTest, InstanceWithParameters) {
    Random rnd((unsigned int)time(0));
    double rx = rnd.nextReal();
    double ry = rnd.nextReal();
    double rz = rnd.nextReal();

    Normal3D n(rx, ry, rz);
    EXPECT_EQ(rx, n.x());
    EXPECT_EQ(ry, n.y());
    EXPECT_EQ(rz, n.z());
}

TEST_F(Normal3DTest, CopyAndAssign) {
    Random rnd((unsigned int)time(0));
    double rx = rnd.nextReal();
    double ry = rnd.nextReal();
    double rz = rnd.nextReal();
    Normal3D n0(rx, ry, rz);

    Normal3D n1(n0);
    EXPECT_EQ(rx, n1.x());
    EXPECT_EQ(ry, n1.y());
    EXPECT_EQ(rz, n1.z());

    Normal3D n2;
    n2 = n0;
    EXPECT_EQ(rx, n2.x());
    EXPECT_EQ(ry, n2.y());
    EXPECT_EQ(rz, n2.z());
}

TEST_F(Normal3DTest, InstanceWithVector3D) {
    Random rnd((unsigned int)time(0));
    double rx = rnd.nextReal();
    double ry = rnd.nextReal();
    double rz = rnd.nextReal();
    
    Vector3D v(rx, ry, rz);
    Normal3D n(v);
    EXPECT_EQ(rx, n.x());
    EXPECT_EQ(ry, n.y());
    EXPECT_EQ(rz, n.z());
}

TEST_F(Normal3DTest, AddAndSubtract) {
    Random rnd((unsigned int)time(0));
    Normal3D n1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    Normal3D n2(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    Normal3D n3 = n1 + n2;
    EXPECT_DOUBLE_EQ(n1.x() + n2.x(), n3.x());
    EXPECT_DOUBLE_EQ(n1.y() + n2.y(), n3.y());
    EXPECT_DOUBLE_EQ(n1.z() + n2.z(), n3.z());

    Normal3D n4 = n1 - n2;
    EXPECT_DOUBLE_EQ(n1.x() - n2.x(), n4.x());
    EXPECT_DOUBLE_EQ(n1.y() - n2.y(), n4.y());
    EXPECT_DOUBLE_EQ(n1.z() - n2.z(), n4.z());
}

TEST_F(Normal3DTest, MultipleAndDivision) {
    Random rnd((unsigned int)time(0));
    Normal3D n1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    double   s = rnd.nextReal();
    while (s == 0.0) s = rnd.nextReal();

    Normal3D n2 = n1 * s;
    EXPECT_DOUBLE_EQ(n1.x() * s, n2.x());
    EXPECT_DOUBLE_EQ(n1.y() * s, n2.y());
    EXPECT_DOUBLE_EQ(n1.z() * s, n2.z());

    Normal3D n3 = s * n1;
    EXPECT_DOUBLE_EQ(n1.x() * s, n3.x());
    EXPECT_DOUBLE_EQ(n1.y() * s, n3.y());
    EXPECT_DOUBLE_EQ(n1.z() * s, n3.z());

    Normal3D n4 = n1 / s;
    EXPECT_DOUBLE_EQ(n1.x() / s, n4.x());
    EXPECT_DOUBLE_EQ(n1.y() / s, n4.y());
    EXPECT_DOUBLE_EQ(n1.z() / s, n4.z());

    double t = 0.0;
    ASSERT_DEATH(n1 / t, "");
}

TEST_F(Normal3DTest, NormAndNormalize) {
    Random rnd((unsigned int)time(0));
    Normal3D n1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    double nrm = n1.x() * n1.x() + n1.y() * n1.y() + n1.z() * n1.z();
    EXPECT_DOUBLE_EQ(nrm, n1.squaredNorm());

    nrm = sqrt(nrm);
    EXPECT_DOUBLE_EQ(nrm, n1.norm());

    if (nrm != 0.0) {
        Normal3D n2 = n1.normalized();
        EXPECT_DOUBLE_EQ(n1.x() / nrm, n2.x());
        EXPECT_DOUBLE_EQ(n1.y() / nrm, n2.y());
        EXPECT_DOUBLE_EQ(n1.z() / nrm, n2.z());
    } else {
        ASSERT_DEATH(n1.normalized(), "");
    }
}
