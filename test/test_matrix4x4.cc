#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

//! Fixture class.
class Matrix4x4Test : public ::testing::Test {
protected:
    Matrix4x4Test() {}
    virtual ~Matrix4x4Test() {}
};

TEST_F(Matrix4x4Test, DefaultInstance) {
    Matrix4x4 mat;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                EXPECT_DOUBLE_EQ(1.0, mat(i, j)) <<
                    "(" << i << ", " << j << ") is wrong.";
            } else {
                EXPECT_DOUBLE_EQ(0.0, mat(i, j)) <<
                    "(" << i << ", " << j << ") is wrong.";
            }
        }
    }
}

TEST_F(Matrix4x4Test, CopyAndAssignment) {
    Random rnd;
    Matrix4x4 m1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    // Copy constructor
    Matrix4x4 m2(m1);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            EXPECT_EQ(m1(i, j), m2(i, j));
        }
    }

    // Assignment
    Matrix4x4 m3;
    m3 = m1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            EXPECT_EQ(m1(i, j), m3(i, j));
        }
    }
}

TEST_F(Matrix4x4Test, SpecifyElementInstance) {
    Matrix4x4 m1( 0.0,  1.0,  2.0,  3.0,
                  4.0,  5.0,  6.0,  7.0,
                  8.0,  9.0, 10.0, 11.0,
                 12.0, 13.0, 14.0, 15.0);

    int k = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++, k++) {
            EXPECT_DOUBLE_EQ(k, m1(i, j)) <<
                "(" << i << ", " << j << ") is wrong.";
        }
    }

    double m[4][4] = {
        { 15.0, 14.0, 13.0, 12.0 },
        { 11.0, 10.0,  9.0,  8.0 },
        {  7.0,  6.0,  5.0,  4.0 },
        {  3.0,  2.0,  1.0,  0.0 }
    };

    Matrix4x4 m2(m);
    k = 15;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++, k--) {
            EXPECT_DOUBLE_EQ(k, m2(i, j)) <<
                "(" << i << ", " << j << ") is wrong.";
        }
    }
}

TEST_F(Matrix4x4Test, Transposition) {
    Random rnd;
    Matrix4x4 m1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    Matrix4x4 m2 = m1.transposed();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            EXPECT_EQ(m1(i, j), m2(j, i));
        }
    }
}

TEST_F(Matrix4x4Test, Multiplication) {
    Random rnd;
    Matrix4x4 m1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    Matrix4x4 m2(rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal());
    
    Matrix4x4 m3 = m1 * m2;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double val = 0.0;
            for (int k = 0; k < 4; k++) {
                val += m1(i, k) * m2(k, j);
            }
            EXPECT_EQ(val, m3(i, j));
        }
    }
}

TEST_F(Matrix4x4Test, Inversion) {
    Matrix4x4 m1(1.0, 1.0, 1.0, 1.0,
                 1.0, 1.0, 1.0, 2.0,
                 1.0, 1.0, 2.0, 3.0,
                 1.0, 2.0, 3.0, 4.0);

    Matrix4x4 m2;
    ASSERT_NO_FATAL_FAILURE(m2 = m1.inverted());
    
    Matrix4x4 m3 = m1 * m2;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                EXPECT_NEAR(1.0, m3(i, j), 1.0e-12) <<
                    "(" << i << ", " << j << ") is wrong.";
            } else {
                EXPECT_NEAR(0.0, m3(i, j), 1.0e-12) <<
                    "(" << i << ", " << j << ") is wrong.";
            }
        }
    }

    m3 = m2 * m1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                EXPECT_NEAR(1.0, m3(i, j), 1.0e-12) <<
                    "(" << i << ", " << j << ") is wrong.";
            } else {
                EXPECT_NEAR(0.0, m3(i, j), 1.0e-12) <<
                    "(" << i << ", " << j << ") is wrong.";
            }
        }
    }
 
    // Singular matrix
    Matrix4x4 m4( 0.0,  1.0,  2.0,  3.0,
                  4.0,  5.0,  6.0,  7.0,
                  8.0,  9.0, 10.0, 11.0,
                 12.0, 13.0, 14.0, 15.0);
    ASSERT_DEATH(m4.inverted(), "");
}

TEST_F(Matrix4x4Test, EqualAndNotEqual) {
    Random rnd;
    Matrix4x4 m1(rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal(),
                 rnd.nextReal(), rnd.nextReal(), rnd.nextReal(), rnd.nextReal());

    Matrix4x4 m2 = m1;
    EXPECT_TRUE(m1 == m2);
    EXPECT_FALSE(m1 != m2);
}

TEST_F(Matrix4x4Test, InvalidAccess) {
    Matrix4x4 m;
    ASSERT_DEATH(m(0, 4), "");
    ASSERT_DEATH(m(4, 0), "");
    ASSERT_DEATH(m(-1, 0), "");
    ASSERT_DEATH(m(0, -1), "");
}
