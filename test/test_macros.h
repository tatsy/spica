#ifndef _SPICA_TEST_MACROS_H_
#define _SPICA_TEST_MACROS_H_

#include <gtest/gtest.h>

#include "../include/spica.h"

inline void EXPECT_EQ_VEC(const Vector3& expected, const Vector3& actual) {
    if (expected.x() != actual.x() ||
        expected.y() != actual.y() ||
        expected.z() != actual.z()) {
        FAIL() << "expected: " << expected.toString() << std::endl
               << "  actual: " << actual.toString()   << std::endl;
    }
    SUCCEED();
}

#endif  // _SPICA_TEST_MACROS_H_
