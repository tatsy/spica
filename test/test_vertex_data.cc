#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

class VertexDataTest : public ::testing::Test {
protected:
    VertexDataTest() {}
    virtual ~VertexDataTest() {}
};

TEST_F(VertexDataTest, DefaultInstance) {
    VertexData vd;

    EXPECT_EQ(0.0, vd.pos().x());
    EXPECT_EQ(0.0, vd.pos().y());
    EXPECT_EQ(0.0, vd.pos().z());

    EXPECT_EQ(0.0, vd.color().red());
    EXPECT_EQ(0.0, vd.color().green());
    EXPECT_EQ(0.0, vd.color().blue());

    EXPECT_EQ(0.0, vd.normal().x());
    EXPECT_EQ(0.0, vd.normal().y());
    EXPECT_EQ(0.0, vd.normal().z());

    EXPECT_EQ(INFTY, vd.texcoord().x());
    EXPECT_EQ(INFTY, vd.texcoord().y());

    EXPECT_FALSE(vd.isTextured());
}

TEST_F(VertexDataTest, Instance) {
    const int trials = 100;
    Random rng((unsigned int)time(0));

    for (int t = 0; t < trials; t++) {
        Vector3D pos(rng.nextReal() * 2.0 - 1.0,
                     rng.nextReal() * 2.0 - 1.0,
                     rng.nextReal() * 2.0 - 1.0);
        Color color(rng.nextReal(), rng.nextReal(), rng.nextReal());
        Vector3D normal(rng.nextReal() * 2.0 - 1.0,
                        rng.nextReal() * 2.0 - 1.0,
                        rng.nextReal() * 2.0 - 1.0);
        Vector2D texcoord(rng.nextReal(), rng.nextReal());

        VertexData vd(pos, color, normal, texcoord);
    
        EXPECT_EQ(pos.x(), vd.pos().x());
        EXPECT_EQ(pos.y(), vd.pos().y());
        EXPECT_EQ(pos.z(), vd.pos().z());

        EXPECT_EQ(color.red(),   vd.color().red());
        EXPECT_EQ(color.green(), vd.color().green());
        EXPECT_EQ(color.blue(),  vd.color().blue());

        EXPECT_EQ(normal.x(), vd.normal().x());
        EXPECT_EQ(normal.y(), vd.normal().y());
        EXPECT_EQ(normal.z(), vd.normal().z());

        EXPECT_EQ(texcoord.x(), vd.texcoord().x());
        EXPECT_EQ(texcoord.y(), vd.texcoord().y());

        EXPECT_TRUE(vd.isTextured());
    }
}

TEST_F(VertexDataTest, Copy) {
    const int trials = 100;
    Random rng((unsigned int)time(0));

    for (int t = 0; t < trials; t++) {
        Vector3D pos(rng.nextReal() * 2.0 - 1.0,
            rng.nextReal() * 2.0 - 1.0,
            rng.nextReal() * 2.0 - 1.0);
        Color color(rng.nextReal(), rng.nextReal(), rng.nextReal());
        Vector3D normal(rng.nextReal() * 2.0 - 1.0,
            rng.nextReal() * 2.0 - 1.0,
            rng.nextReal() * 2.0 - 1.0);
        Vector2D texcoord(rng.nextReal(), rng.nextReal());

        VertexData vd0(pos, color, normal, texcoord);

        VertexData vd = vd0;

        EXPECT_EQ(pos.x(), vd.pos().x());
        EXPECT_EQ(pos.y(), vd.pos().y());
        EXPECT_EQ(pos.z(), vd.pos().z());

        EXPECT_EQ(color.red(), vd.color().red());
        EXPECT_EQ(color.green(), vd.color().green());
        EXPECT_EQ(color.blue(), vd.color().blue());

        EXPECT_EQ(normal.x(), vd.normal().x());
        EXPECT_EQ(normal.y(), vd.normal().y());
        EXPECT_EQ(normal.z(), vd.normal().z());

        EXPECT_EQ(texcoord.x(), vd.texcoord().x());
        EXPECT_EQ(texcoord.y(), vd.texcoord().y());

        EXPECT_TRUE(vd.isTextured());
    }
}

TEST_F(VertexDataTest, Assign) {
    const int trials = 100;
    Random rng((unsigned int)time(0));

    for (int t = 0; t < trials; t++) {
        Vector3D pos(rng.nextReal() * 2.0 - 1.0,
            rng.nextReal() * 2.0 - 1.0,
            rng.nextReal() * 2.0 - 1.0);
        Color color(rng.nextReal(), rng.nextReal(), rng.nextReal());
        Vector3D normal(rng.nextReal() * 2.0 - 1.0,
            rng.nextReal() * 2.0 - 1.0,
            rng.nextReal() * 2.0 - 1.0);
        Vector2D texcoord(rng.nextReal(), rng.nextReal());

        VertexData vd0(pos, color, normal, texcoord);

        VertexData vd;
        vd = vd0;

        EXPECT_EQ(pos.x(), vd.pos().x());
        EXPECT_EQ(pos.y(), vd.pos().y());
        EXPECT_EQ(pos.z(), vd.pos().z());

        EXPECT_EQ(color.red(), vd.color().red());
        EXPECT_EQ(color.green(), vd.color().green());
        EXPECT_EQ(color.blue(), vd.color().blue());

        EXPECT_EQ(normal.x(), vd.normal().x());
        EXPECT_EQ(normal.y(), vd.normal().y());
        EXPECT_EQ(normal.z(), vd.normal().z());

        EXPECT_EQ(texcoord.x(), vd.texcoord().x());
        EXPECT_EQ(texcoord.y(), vd.texcoord().y());

        EXPECT_TRUE(vd.isTextured());
    }
}
