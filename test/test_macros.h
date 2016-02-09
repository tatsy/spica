#ifndef _SPICA_TEST_MACROS_H_
#define _SPICA_TEST_MACROS_H_

#include <gtest/gtest.h>
#include <cmath>

#include "../include/spica.h"

const double tol = 0.001;

template <class Vec1, class Vec2>
inline void EXPECT_EQ_VEC_(const Vec1& expected, const Vec2& actual) {
    if (std::abs((expected).x() - (actual).x()) >= tol ||
        std::abs((expected).y() - (actual).y()) >= tol ||
        std::abs((expected).z() - (actual).z()) >= tol) {
        ADD_FAILURE() << "expected: " << (expected).toString() << std::endl 
                      << "  actual: " << (actual).toString()   << std::endl;
    }
    SUCCEED();
}

#define EXPECT_EQ_VEC(expected, actual) EXPECT_EQ_VEC_(expected, actual);

#endif  // _SPICA_TEST_MACROS_H_
