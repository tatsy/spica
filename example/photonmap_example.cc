#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char** argv) {
    std::cout << "*** spica: Photon mapping ***" << std::endl;

    const int width = argc >= 2 ? atoi(argv[1]) : 320;
    const int height = argc >= 3 ? atoi(argv[2]) : 240;
    const int samplePerPixel = argc >= 4 ? atoi(argv[3]) : 32;

    std::cout << "      width: " << width << std::endl;
    std::cout << "     height: " << height << std::endl;
    std::cout << "  sample/px: " << samplePerPixel << std::endl << std::endl;

    Scene scene;
    Camera camera;
    cornellBoxBunny(scene, camera, width, height);

    Random rng = Random::getRNG();

    const int numPhotons = 50000;

    Timer timer;
    PMRenderer renderer;
    renderer.buildPM(scene, camera, rng, numPhotons);
}
