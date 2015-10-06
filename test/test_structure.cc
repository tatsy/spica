#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;
using namespace structure;

class StructureTest : public ::testing::Test {
protected:
    StructureTest() {}
    virtual ~StructureTest() {}
};

TEST_F(StructureTest, Instance) {
    Mesh mesh;
    EXPECT_EQ(0, mesh.numVertes());
    EXPECT_EQ(0, mesh.numFaces());
}