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

const double hal2[10] = { 0.0, 0.5, 0.25, 0.75, 0.125, 0.625, 0.375, 0.875, 1/16.0, 9/16.0 };
const double hal3[10] = { 0.0, 1/3.0, 2/3.0, 1/9.0, 4/9.0, 7/9.0, 2/9.0, 5/9.0, 8/9.0, 1/27.0 };

TEST_F(RandomTest, HaltonTest) {
    Halton hal(2, false);
    Stack<double> stk;
    for (int i = 0; i < 10; i++) {
        hal.request(&stk, 2);
        EXPECT_FLOAT_EQ(hal3[i], stk.pop());
        EXPECT_FLOAT_EQ(hal2[i], stk.pop());
    }
}
