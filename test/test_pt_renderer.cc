#include "gtest/gtest.h"

#include "../include/spica.h"
using namespace spica;

#include "test_macros.h"

namespace {
    const int width  = 120;
    const int height =  90;
    const int spp    =  16;

    const std::string comp_path = DATA_DIR + "compare.bmp";
}

TEST(PTRendererTest, RenderTest) {
    Scene scene;
    Camera camera;
    cornellBox(&scene, &camera, width, height);
    
    PathTracingRenderer renderer;
    renderer.render(scene, camera, spp, QUASI_MONTE_CARLO);
}