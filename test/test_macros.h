#ifndef _SPICA_TEST_MACROS_H_
#define _SPICA_TEST_MACROS_H_

inline void EXPECT_EQ_VEC(const Vector3& v1, const Vector3& v2) {
    EXPECT_EQ(v1.x(), v2.x());
    EXPECT_EQ(v1.y(), v2.y());
    EXPECT_EQ(v1.z(), v2.z());
}

#endif  // _SPICA_TEST_MACROS_H_
