#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char **argv) {
    std::cout << "Bidirectional path tracing" << std::endl << std::endl;

    Scene scene;
    Camera camera;
    cornellBoxBunny(scene, camera, 320, 240);

    Random rng = Random::getRNG();
    const int samplePerPixel = 32;

    BPTRenderer renderer;
    renderer.render(scene, camera, rng, samplePerPixel);

    return 0;
}
