#include "gtest/gtest.h"

#include "../include/spica.h"
#include "../src/renderer/renderer_helper.h"
using namespace spica;

#include "test_macros.h"

TEST(RendererHelperTest, TotalRefTest) {
    Vector3D position(0.0, 0.0, 0.0);
    Vector3D in = Vector3D(-1.0, 0.0, -1.0).normalized();
    Vector3D normal(0.0, 1.0, 0.0);

    Vector3D reflectDir, reflactDir;
    double fresnelRef, fresnelTransmit;
    EXPECT_FALSE(helper::isTotalRef(true, position, in, normal, normal, &reflectDir, &reflactDir, &fresnelRef, &fresnelTransmit));

    Vector3D refdirExp = Vector3D::reflect(in, normal);
    EXPECT_EQ_VEC(refdirExp, reflectDir);
}