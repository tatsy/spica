#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

class RandomTest : public ::testing::Test {
protected:
    RandomTest() 
    {
    }

    ~RandomTest()
    {
    }

protected:
    static const int nTest = 100;
    static const int nLoop = 50000;
    static Random rng;
};

Random RandomTest::rng = Random();

TEST_F(RandomTest, RandInt) {
    int nBins = 10;
    int *countUp = new int[10];
    memset(countUp, 0, sizeof(int) * nBins);
    for (int i = 0; i < nLoop; i++) {
        countUp[rng.nextInt(nBins)]++;
    }

    for (int i = 0; i < nBins; i++) {
        EXPECT_LT(abs(countUp[i] - nLoop / nBins), nLoop / (nBins * nBins));
    }

    delete[] countUp;
}

TEST_F(RandomTest, RandReal) {
    for (int t = 0; t < nTest; t++) {
        double accum = 0.0;
        for (int i = 0; i < nLoop; i++) {
            double rx = rng.nextReal();
            double ry = rng.nextReal();
            if (hypot(rx, ry) <= 1.0) {
                accum += 1.0;
            }
        }
        ASSERT_NEAR(accum / nLoop, PI / 4.0, 1.0e-2);
    }
}

TEST_F(RandomTest, HaltonTest) {
    Halton halton;
    EXPECT_EQ(0.50, halton.nextReal(0, 1));
    EXPECT_EQ(0.25, halton.nextReal(0, 2));
    EXPECT_EQ(0.75, halton.nextReal(0, 3));
    EXPECT_EQ(0.125, halton.nextReal(0, 4));
    EXPECT_EQ(0.625, halton.nextReal(0, 5));
    EXPECT_EQ(0.375, halton.nextReal(0, 6));
}
