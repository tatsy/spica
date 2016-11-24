#include "gtest/gtest.h"

#include "../include/spica.h"
#include "../src/renderer/renderer_helper.h"
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

        double rmse = 0.0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                rmse += (result(x, y) - ref(x, y)).squaredNorm();
            }
        }
        rmse = sqrt(rmse) / (width * height);
        printf("RMSE: %.8f\n", rmse);
        EXPECT_LT(rmse, 0.01);
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

TEST_F(RendererTest, MLTTest) {
    MLTRenderer renderer(&result);
    Random rnd = Random();
    renderer.render(scene, camera, rnd, spp, width * height, 32);
    TestMain();
}
