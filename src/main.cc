#include <iostream>

#include "rainy.h"
using namespace rainy;

int main(int argc, char **argv) {
    std::cout << "Path tracing" << std::endl << std::endl;

    Renderer renderer(200, 200, 16, 2);
    renderer.render();

    return 0;
}
