#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

class RendererTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        ref.loadBMP(DATA_DIR + "reference.bmp");
        width = ref.width();
        height = ref.height();
        cornellBox(&scene, &camera, width, height);

        result.fill(Color(0.0, 0.0, 0.0));
    }

    void TestMain() {
        EXPECT_EQ(ref.width(), result.width());
        EXPECT_EQ(ref.height(), result.height());

        double mean = 0.0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                mean += (result(x, y) - ref(x, y)).norm();
            }
        }
        mean /= (width * height);

        double std = 0.0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double delta = (result(x, y) - ref(x, y)).norm() - mean;
                std += delta * delta;
            }
        }
        std = sqrt(std / (width * height));

        EXPECT_LT(mean, 0.5);
        EXPECT_LT(std,  0.5);
    }

protected:
    Image ref;
    Image result;
    const int spp = 32;
    int width;
    int height;
    Scene scene;
    Camera camera;
};


TEST_F(RendererTest, PTTest) {    
    PathTracingRenderer renderer(&result);
    renderer.render(scene, camera, spp, QUASI_MONTE_CARLO);
    TestMain();
}

TEST_F(RendererTest, BDPTTest) {
    BDPTRenderer renderer(&result);
    renderer.render(scene, camera, spp, QUASI_MONTE_CARLO);
    TestMain();
}
