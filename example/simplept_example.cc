#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char **argv) {
    std::cout << "Path tracing" << std::endl << std::endl;

    const int width = 320;
    const int height = 240;

    Scene scene;
    Camera camera;
    cornellBoxBunny(scene, camera, width, height);

    Random rng = Random::getRNG();
    const int samplePerPixel = 1024;

    PTRenderer renderer;
    renderer.render(scene, camera, rng, samplePerPixel);

    return 0;
}
