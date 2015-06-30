#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

class RendererTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        ref.loadBMP(DATA_DIR + "reference.bmp");
        width = ref.width();
        height = ref.height();
        cornellBox(&scene, &camera, width, height);
    }

    virtual void SetUp() {
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
    static Image ref;
    static Image result;
    static const int spp;
    static int width;
    static int height;
    static Scene scene;
    static Camera camera;
};

Image RendererTest::ref = Image();
Image RendererTest::result = Image();
const int RendererTest::spp = 32;
int RendererTest::width = 0;
int RendererTest::height = 0;
Scene RendererTest::scene;
Camera RendererTest::camera;

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
