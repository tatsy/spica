#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include <iostream>
#include <fstream>

#include "test_macros.h"

const int nTrial = 100;

TEST(SamplerTest, DiskSampleTest) {
    Vector3D C(0.0, 0.0, 0.0);
    Vector3D N(0.0, 0.0, 1.0);
    double R = 5.0;
    Disk disk(C, N, R);

    for (int i = 0; i < nTrial; i++) {
        Vector3D position;
        Vector3D normal;
        sampler::onDisk(disk, &position, &normal);

        EXPECT_EQ_VEC(N, normal);
        EXPECT_LE((C - position).norm(), R);
        EXPECT_EQ(0.0, Vector3D::dot(C - position, N));
    }
}

TEST(SamplerTest, PoissonDiskTest) {
    /*
    const double mindist = 0.1;
    const double maxdist = 0.5;
    Trimesh trimesh(kDataDirectory + "bunny.ply");

    std::vector<Vector3D> points;
    std::vector<Vector3D> normals;
    sampler::poissonDisk(trimesh, mindist, maxdist, &points, &normals);

    for (int i = 0; i < points.size(); i++) {
        for (int j =  i + 1; j < points.size(); j++) {
            EXPECT_LE(mindist, (points[i] - points[j]).norm());
        }
    }
    */
}
