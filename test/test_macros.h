#ifndef _SPICA_TEST_MACROS_H_
#define _SPICA_TEST_MACROS_H_

#include <gtest/gtest.h>
#include <cmath>

#include "../include/spica.h"

#define EXPECT_EQ_VEC(expected, actual) \
do { \
    if (std::abs(expected.x() - actual.x()) >= EPS || \
        std::abs(expected.y() - actual.y()) >= EPS || \
        std::abs(expected.z() - actual.z()) >= EPS) { \
        FAIL() << "expected: " << expected.toString() << std::endl  \
               << "  actual: " << actual.toString()   << std::endl; \
    } \
    SUCCEED(); \
} while (false)
#endif  // _SPICA_TEST_MACROS_H_
