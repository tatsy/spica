#ifdef _MSC_VER
#pragma once
#endif

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

#include "scene.h"
#include "camera.h"

#include "../utils/vector3d.h"
#include "../utils/color.h"

namespace spica {

    namespace helper {

        // Calculate u and v axes from w vector
        void SPICA_RENDERER_HELPER_DLL calcLocalCoords(const Vector3D& w, Vector3D* u, Vector3D* v);

        bool SPICA_RENDERER_HELPER_DLL checkTotalReflection(const bool isIncoming,
                                                            const Vector3D& in,
                                                            const Vector3D& normal,
                                                            const Vector3D& orientNormal,
                                                            Vector3D* reflectDir,
                                                            Vector3D* refractDir,
                                                            double* fresnelRef,
                                                            double* fresnelTransmit);

        // Standard radiance simulator
        // @param[in] scene: rendered scene
        // @param[in] ray: ray casted from camera
        // @param[in] rseq: random number sequence
        // @param[in] depth: depth of recursion
        // @param[in] depthLimit: maximum depth of recursion
        // @param[in] depthMin: depth in which recursion begin to be terminated with Russian roulette
        Color SPICA_RENDERER_HELPER_DLL radiance(const Scene& scene, const Ray& ray, RandomSeq& rseq, const int depth, const int depthLimit = 64, const int depthMin = 6);
    }

}

#endif  // _SPICA_RENDERER_HELPER_H_