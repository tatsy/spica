#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

// ------------------------------
// Trimesh class test
// ------------------------------
TEST(TrimeshTest, BoxIntersection) {
    Trimesh trimesh;
    trimesh.load(DATA_DIR + "box.ply");
    trimesh.buildKdTreeAccel();

    Ray ray(Vector3(0.0, 0.0, 100.0), Vector3(0.0, 0.0, -1.0));
    Hitpoint hitpoint;
    EXPECT_TRUE(trimesh.intersect(ray, &hitpoint));
    EXPECT_EQ(99.5, hitpoint.distance());
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 0.5), hitpoint.position());
    EXPECT_EQ_VEC(Vector3(0.0, 0.0, 1.0), hitpoint.normal());
}

TEST(TrimeshTest, BunnyIntersection) {
    Trimesh trimesh;
    trimesh.load(DATA_DIR + "bunny.ply");
    trimesh.buildKdTreeAccel();

    Ray ray(Vector3(0.0, 0.0, 100.0), Vector3(0.0, 0.0, -1.0));

    Hitpoint hpGT;
    for (int i = 0; i < trimesh.numFaces(); i++) {
        Triangle tri = trimesh.getTriangle(i);
        Hitpoint hpTemp;
        if (tri.intersect(ray, &hpTemp)) {
            if (hpGT.distance() > hpTemp.distance()) {
                hpGT = hpTemp;
            }
        }
    }

    Hitpoint hitpoint;
    EXPECT_TRUE(trimesh.intersect(ray, &hitpoint));
    EXPECT_EQ(hpGT.distance(), hitpoint.distance());
}