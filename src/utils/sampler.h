#ifndef SPICA_SAMPLER_H_
#define SPICA_SAMPLER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SAMPLER_EXPORT
        #define SPICA_SAMPLER_DLL __declspec(dllexport)
    #else
        #define SPICA_SAMPLER_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_SAMPLER_DLL
#endif

#include "random.h"
#include "vector3.h"
#include "../geometry/geometry.h"

namespace spica {

    namespace sampler {
    
        Vector3 onDisk(const Disk& disk);

        Vector3 onQuad(const Quad& quad);
    
    }  // namespace sampler

}  // namespace spica

#endif  // SPICA_SAMPLER_H_
