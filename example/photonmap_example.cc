#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char** argv) {
    const int width   = argc >= 2 ? atoi(argv[1]) : 400;
    const int height  = argc >= 3 ? atoi(argv[2]) : 300;
    const int samples = argc >= 4 ? atoi(argv[3]) : 32;

    std::cout << "--- photon mapping ---" << std::endl;
    std::cout << "    width: " << width   << std::endl;
    std::cout << "   height: " << height  << std::endl;
    std::cout << "  samples: " << samples << std::endl << std::endl;

    Scene scene;
    Camera camera;
    cornellBox(&scene, &camera, width, height);
    scene.computeLightPdfs();

    const int numPhotons = 1000000;
    const int gatherPhotons = 200;
    const double gatherRadius = 20.0;

    RenderParameters params(samples);
    params.castPhotons(numPhotons);
    params.gatherPhotons(gatherPhotons);
    params.gatherRadius(gatherRadius);

    Timer timer;
    timer.start();

    PMRenderer renderer;
    renderer.render(scene, camera, params);
    printf("Time: %f sec\n", timer.stop());
}
