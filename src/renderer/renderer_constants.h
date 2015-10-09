#ifndef _SPICA_RENDERER_CONSTANTS_H_
#define _SPICA_RENDERER_CONSTANTS_H_

#include "ray.h"
#include "../scenes/scene.h"
#include "../camera/camera.h"

namespace spica {

    enum RandomType {

        PSEUDO_RANDOM_TWISTER = 0x01,  // Monte Carlo integrater using Mersenne twister
        QUASI_MONTE_CARLO = 0x02,      // Quasi Monte Carlo integrator with randomized Halton

    };

    const double kIorVaccum = 1.0;
    const double kIorObject = 1.5;
    const double kReflectProbability = 0.5;

}

#endif  // _SPICA_RENDERER_CONSTANTS_H_
