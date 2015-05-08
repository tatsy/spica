#include <iostream>

#include "../include/spica.h"
using namespace spica;


int main(int argc, char **argv) {
    std::cout << "Metropolis light transport" << std::endl;

    const int width = 320;
    const int height = 240;

    Scene scene;
    Camera camera;
    cornellBoxBunny(scene, camera, width, height);

    const int numMutate = 8 * width * height;
    const int numMLT    = 8;
    const int maxDepth  = 5;
    Random rng = Random::getRNG();

    MLTRenderer renderer;
    renderer.render(scene, camera, rng, numMLT, numMutate, maxDepth);
}