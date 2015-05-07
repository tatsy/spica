#include <iostream>

#include "../include/spica.h"
using namespace spica;


int main(int argc, char **argv) {
    std::cout << "Metropolis light transport" << std::endl;

    const int width = 320;
    const int height = 240;

    Scene scene;
    Camera camera;
    cornellBox(scene, camera, width, height);

    const int numMutate = 32 * width * height;
    const int numMLT    = 32;
    const int maxDepth  = 5;
    Random rng = Random::getRNG();

    MLTRenderer renderer;
    renderer.render(scene, camera, rng, numMLT, numMutate, maxDepth);
}