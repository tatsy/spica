#include <cstdio>
#include <iostream>

#include "../include/spica.h"
using namespace spica;

int main(int argc, char** argv) {
    const int width   = argc >= 2 ? atoi(argv[1]) : 400;
    const int height  = argc >= 3 ? atoi(argv[2]) : 300;
    const int samples = argc >= 4 ? atoi(argv[3]) : 32;

    std::cout << "--- stochastic progressive photon mapping ---" << std::endl;
    std::cout << "    width: " << width   << std::endl;
    std::cout << "   height: " << height  << std::endl;
    std::cout << "  samples: " << samples << std::endl << std::endl;

    
    Scene scene;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Sampler> sampler = std::make_unique<Random>(0);

    cornellBox(&scene, &camera, Point2i(width, height));
    // kittenBox(&scene, &camera, width, height);
    // kittenEnvmap(&scene, &camera, width, height);
    
    Timer timer;
    timer.start();

    RenderParams params;
    params.set("NUM_SAMPLES", samples);
    params.set("MAX_BOUNCES", 32);
    params.set("CAST_PHOTONS", 200000);
   
    SPPMIntegrator integr(camera, sampler);
    integr.render(scene, params);

    printf("Timer: %f sec\n", timer.stop());}
