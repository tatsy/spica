#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;


int main(int argc, char **argv) {
    const int width   = argc >= 2 ? atoi(argv[1]) : 320;
    const int height  = argc >= 3 ? atoi(argv[2]) : 240;
    const int samples = argc >= 4 ? atoi(argv[3]) : 4;
    const int numMLT  = argc >= 5 ? atoi(argv[4]) : 16;

    std::cout << "--- primary space metropolis light transport ---" << std::endl;
    std::cout << "     width: " << width   << std::endl;
    std::cout << "    height: " << height  << std::endl;
    std::cout << "   samples: " << samples << std::endl;
    std::cout << "  # of MLT: " << numMLT  << std::endl;

    Scene scene;
    Camera camera;
    cornellBoxOcclusion(&scene, &camera, width, height);

    const int numMutate = width * height * samples;
    const int maxDepth  = 8;
    Random rng = Random();

    Timer timer;
    timer.start();
    MLTRenderer renderer;
    renderer.render(scene, camera, rng, numMLT, numMutate, maxDepth);
    printf("Time: %f\n", timer.stop());
}