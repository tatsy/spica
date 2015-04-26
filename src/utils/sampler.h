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

namespace spica {

    class Sampler {
    private:
        static Random rng;
    
    public:
        static Vector3 onDisk(const Vector3& center, const Vector3& normal, double radius);
    
    };

}

#endif  // SPICA_SAMPLER_H_
