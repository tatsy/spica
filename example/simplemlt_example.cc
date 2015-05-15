#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;


int main(int argc, char **argv) {
    std::cout << "*** spica: Metropolis light transport ***" << std::endl;

    const int width  = argc >= 2 ? atoi(argv[1]) : 320;
    const int height = argc >= 3 ? atoi(argv[2]) : 240;
    const int samplePerPixel = argc >= 4 ? atoi(argv[3]) : 8;
    const int numMLT = argc >= 5 ? atoi(argv[4]) : 8;

    std::cout << "      width: " << width << std::endl;
    std::cout << "     height: " << height << std::endl;
    std::cout << "  sample/px: " << samplePerPixel << std::endl;
    std::cout << "   # of MLT: " << numMLT << std::endl;

    Scene scene;
    Camera camera;
    cornellBox(scene, camera, width, height);

    const int numMutate = width * height * samplePerPixel;
    const int maxDepth  = 6;
    Random rng = Random();

    Timer timer;
    timer.start();
    MLTRenderer renderer;
    renderer.render(scene, camera, rng, numMLT, numMutate, maxDepth);
    printf("Time: %f\n", timer.stop());
}