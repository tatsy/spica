#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char **argv) {
    const int width   = argc >= 2 ? atoi(argv[1]) : 400;
    const int height  = argc >= 3 ? atoi(argv[2]) : 300;
    const int samples = argc >= 4 ? atoi(argv[3]) : 32;

    std::cout << "--- BDPT (bi-directional path tracing) ---" << std::endl;
    std::cout << "      width: " << width   << std::endl;
    std::cout << "     height: " << height  << std::endl;
    std::cout << "  sample/px: " << samples << std::endl << std::endl;

    Scene scene;
    Camera camera;
    cornellBox(&scene, &camera, width, height);
    scene.computeLightPdfs();

    RenderParameters params(samples);

    Timer timer;
    timer.start();
    BDPTRenderer renderer;
    renderer.render(scene, camera, params);

    printf("Timer: %f sec\n", timer.stop());
}
