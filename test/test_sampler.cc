#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

const int nTrial = 100;

TEST(SamplerTest, DiskSampleTest) {
    Vector3 C(0.0, 0.0, 0.0);
    Vector3 N(0.0, 0.0, 1.0);
    double R = 5.0;
    Disk disk(C, N, R);

    for (int i = 0; i < nTrial; i++) {
        Vector3 position;
        Vector3 normal;
        sampler::onDisk(disk, &position, &normal);

        EXPECT_EQ_VEC(N, normal);
        EXPECT_LE((C - position).norm(), R);
        EXPECT_EQ(0.0, Vector3::dot(C - position, N));
    }
}
