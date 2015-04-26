#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char **argv) {
    std::cout << "Path tracing" << std::endl << std::endl;

    const int width = 640;
    const int height = 480;

    Scene scene;
    Camera camera;
    cornellBox(scene, camera, width, height);


    Random rng = Random::getRNG();
    const int samplePerPixel = 512;

    PTRenderer renderer;
    renderer.render(scene, camera, rng, samplePerPixel);

    return 0;
}
