#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char** argv) {
    std::cout << "*** spica: Photon mapping ***" << std::endl;

    const int width = argc >= 2 ? atoi(argv[1]) : 320;
    const int height = argc >= 3 ? atoi(argv[2]) : 240;
    const int samplePerPixel = argc >= 4 ? atoi(argv[3]) : 16;

    std::cout << "      width: " << width << std::endl;
    std::cout << "     height: " << height << std::endl;
    std::cout << "  sample/px: " << samplePerPixel << std::endl << std::endl;

    Scene scene;
    Camera camera;
    cornellBox(&scene, &camera, width, height);

    const int numPhotons = 1000000;
    const int gatherPhotons = 200;
    const double gatherRadius = 20.0;

    PMRenderer renderer;
    PMParams params(numPhotons, gatherPhotons, gatherRadius);

    Timer timer;
    timer.start();
    renderer.render(scene, camera, samplePerPixel, params, QUASI_MONTE_CARLO);
    printf("Time: %f sec\n", timer.stop());
}
