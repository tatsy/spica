#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char** argv) {
    std::cout << "*** spica: Subsurface scattering (SPPM) ***" << std::endl;

    const int width = argc >= 2 ? atoi(argv[1]) : 320;
    const int height = argc >= 3 ? atoi(argv[2]) : 240;
    const int samplePerPixel = argc >= 4 ? atoi(argv[3]) : 32;

    std::cout << "      width: " << width << std::endl;
    std::cout << "     height: " << height << std::endl;
    std::cout << "  sample/px: " << samplePerPixel << std::endl << std::endl;

    Scene scene;
    Camera camera;
    kittenEnvmap(&scene, &camera, width, height);

    const int numPhotons = 2000000;

    BSSRDF bssrdf = DiffusionBSSRDF::factory(1.0e-4, 10.0, 1.3);

    SubsurfaceSPPMRenderer renderer;

    Timer timer;
    timer.start();
    renderer.render(scene, camera, bssrdf, samplePerPixel, numPhotons, QUASI_MONTE_CARLO);
    printf("Time: %f sec\n", timer.stop());
}
