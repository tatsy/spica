#include "gtest/gtest.h"

#include "spica.h"
using namespace spica;

//! Fixture class.
class TransformTest : public ::testing::Test {
protected:
    TransformTest() {}
    virtual ~TransformTest() {}
};

TEST_F(TransformTest, DefaultInstance) {
    Transform t;
    const Matrix4x4& m = t.getMat();
    const Matrix4x4& mInv = t.getInvMat();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                EXPECT_EQ(1.0, m(i, j));
                EXPECT_EQ(1.0, mInv(i, j));
            } else {
                EXPECT_EQ(0.0, m(i, j));
                EXPECT_EQ(0.0, mInv(i, j));
            }
        }
    }
}

TEST_F(TransformTest, InstanceWithMatrix) {
    Random rnd;

    double elems[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            elems[i][j] = rnd.nextReal();
        }
    }

    Matrix4x4 mat(elems);
    Matrix4x4 inv = mat.inverted();

    Transform t(mat);
    const Matrix4x4& m = t.getMat();
    const Matrix4x4& mInv = t.getInvMat();
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            EXPECT_DOUBLE_EQ(mat(i, j), m(i, j));
            EXPECT_DOUBLE_EQ(inv(i, j), mInv(i, j));
        }
    }
}

TEST_F(TransformTest, CopyAndAssign) {
    Random rnd;

    double elems[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            elems[i][j] = rnd.nextReal();
        }
    }
    Matrix4x4 mat(elems);

    Transform t1(mat);
    Transform t2(t1);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            EXPECT_EQ(t1.getMat()(i, j), t2.getMat()(i, j));
        }
    }

    Transform t3;
    t3 = t1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            EXPECT_EQ(t1.getMat()(i, j), t3.getMat()(i, j));
        }
    }
}

TEST_F(TransformTest, IsIdentity) {
    Transform t;
    EXPECT_TRUE(t.isIdentity());
}

TEST_F(TransformTest, EqualAndNotEqual) {
    Random rnd;

    double elems[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            elems[i][j] = rnd.nextReal();
        }
    }
    Matrix4x4 mat(elems);

    Transform t1(mat);
    Transform t2(t1);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            EXPECT_EQ(t1.getMat()(i, j), t2.getMat()(i, j));
        }
    }

    EXPECT_TRUE(t1 == t2);
    EXPECT_FALSE(t1 != t2);
}
