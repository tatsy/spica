#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char **argv) {
    std::cout << "*** spica: Monte-Carlor path tracing ***" << std::endl;

    const int width  = argc >= 2 ? atoi(argv[1]) : 320;
    const int height = argc >= 3 ? atoi(argv[2]) : 240;
    const int samplePerPixel = argc >= 4 ? atoi(argv[3]) : 32;
    
    std::cout << "      width: " << width << std::endl;
    std::cout << "     height: " << height << std::endl;
    std::cout << "  sample/px: " << samplePerPixel << std::endl << std::endl;

    Scene scene;
    Camera camera;
    cornellBoxOcclusion(&scene, &camera, width, height);
    // cornellBoxBunny(scene, camera, width, height);
    // litByEnvmap(scene, camera, width, height);

    Random rng = Random();

    Timer timer;
    timer.start();
    PathTracingRenderer renderer;
    renderer.render(scene, camera, samplePerPixel, QUASI_MONTE_CARLO);
    // renderer.render(scene, camera, samplePerPixel, PSEUDO_RANDOM_TWISTER);
    printf("Timer: %f sec\n", timer.stop());

    return 0;
}
