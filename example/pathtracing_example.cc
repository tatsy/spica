#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char **argv) {
    std::cout << "*** spica: uni-directional path tracing ***" << std::endl;

    const int width  = argc >= 2 ? atoi(argv[1]) : 400;
    const int height = argc >= 3 ? atoi(argv[2]) : 300;
    const int spp    = argc >= 4 ? atoi(argv[3]) : 32;
    
    std::cout << "    width: " << width  << std::endl;
    std::cout << "   height: " << height << std::endl;
    std::cout << "  samples: " << spp    << std::endl << std::endl;

    Scene scene;
    Camera camera;
    cornellBox(&scene, &camera, width, height);

    Timer timer;
    timer.start();

    PathTracingRenderer renderer;
    renderer.render(scene, camera, spp, PSEUDO_RANDOM_TWISTER);

    printf("Timer: %f sec\n", timer.stop());

    return 0;
}
