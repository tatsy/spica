#ifndef _SPICA_RENDERER_CONSTANTS_H_
#define _SPICA_RENDERER_CONSTANTS_H_

#include "ray.h"
#include "scene.h"
#include "camera.h"
#include "material.h"

namespace spica {

    enum RandomType {
        PSEUDO_RANDOM_TWISTER = 0x01,  // Monte Carlo integrater using Mersenne twister
        QUASI_MONTE_CARLO = 0x02,      // Quasi Monte Carlo integrator with randomized Halton
    };

}

#endif  // _SPICA_RENDERER_CONSTANTS_H_
