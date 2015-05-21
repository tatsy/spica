#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char** argv) {
    std::cout << "*** spica: Progressive photon mapping ***" << std::endl;

    const int width = argc >= 2 ? atoi(argv[1]) : 640;
    const int height = argc >= 3 ? atoi(argv[2]) : 480;
    const int samplePerPixel = argc >= 4 ? atoi(argv[3]) : 128;

    std::cout << "      width: " << width << std::endl;
    std::cout << "     height: " << height << std::endl;
    std::cout << "  sample/px: "  << samplePerPixel << std::endl << std::endl;

    Scene scene;
    Camera camera;
    cornellBox(scene, camera, width, height);

    Random rng = Random();

    const int numPhotons = 2000000;

    PPMRenderer renderer;

    Timer timer;
    timer.start();
    renderer.render(scene, camera, rng, samplePerPixel, numPhotons);
    printf("Time: %f sec\n", timer.stop());
}
