#ifndef _SPICA_TEST_MACROS_H_
#define _SPICA_TEST_MACROS_H_

inline void EXPECT_EQ_VEC(const Vector3& expected, const Vector3& actual) {
    if (expected.x() != actual.x() ||
        expected.y() != actual.y() ||
        expected.z() != actual.z()) {
        FAIL() << "expected: " << expected << std::endl
               << "  actual: " << actual << std::endl;
    }
    SUCCEED();
}

#endif  // _SPICA_TEST_MACROS_H_
