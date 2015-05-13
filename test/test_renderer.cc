#include "gtest/gtest.h"

#include "../include/spica.h"
#include "../src/renderer/renderer_helper.h"
using namespace spica;

#include "test_macros.h"

TEST(RendererTest, TotalRefTest) {
    Vector3 position(0.0, 0.0, 0.0);
    Vector3 in = Vector3(-1.0, 0.0, -1.0).normalized();
    Vector3 normal(0.0, 1.0, 0.0);

    Vector3 reflectDir, reflactDir;
    double fresnelRef, fresnelTransmit;
    EXPECT_FALSE(helper::isTotalRef(true, position, in, normal, normal, &reflectDir, &reflactDir, &fresnelRef, &fresnelTransmit));

    Vector3 refdirExp = Vector3::reflect(in, normal);
    EXPECT_EQ_VEC(refdirExp, reflectDir);
}