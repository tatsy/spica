#ifndef _SPICA_RENDERER_HELPER_H_
#define _SPICA_RENDERER_HELPER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_RENDERER_HELPER_EXPORT
        #define SPICA_RENDERER_HELPER_DLL __declspec(dllexport)
    #else
        #define SPICA_RENDERER_HELPER_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_RENDERER_HELPER_DLL
#endif

#include "material.h"
#include "scene.h"
#include "camera.h"

#include "../utils/vector3.h"
#include "../utils/color.h"

namespace spica {

    namespace helper {

        bool SPICA_RENDERER_HELPER_DLL isTotalRef(const bool isIncoming,
                                                  const Vector3& position,
                                                  const Vector3& in,
                                                  const Vector3& normal,
                                                  const Vector3& orientNormal,
                                                  Vector3* reflectDir,
                                                  Vector3* refractDir,
                                                  double* fresnelRef,
                                                  double* fresnelTransmit);

        // Standard radiance simulator
        // @param[in] scene: rendered scene
        // @param[in] ray: ray casted from camera
        // @param[in] rng: random number generator
        // @param[in] depth: depth of recursion
        // @param[in] depthLimit: maximum depth of recursion
        // @param[in] depthMin: depth in which recursion begin to be terminated with Russian roulette
        Color radiance(const Scene& scene, const Ray& ray, const Random& rng, const int depth, const int depthLimit = 64, const int depthMin = 5);

        Color radiance(const Scene& scene, const Ray& ray, const Halton& halton, const int sampleID, const int depth, const int depthLimit = 32, const int depthMin = 5);    
    }

}

#endif  // _SPICA_RENDERER_HELPER_H_