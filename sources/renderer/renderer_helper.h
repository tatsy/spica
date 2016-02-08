#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDERER_HELPER_H_
#define _SPICA_RENDERER_HELPER_H_

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/spectrum.h"

namespace spica {

    namespace helper {

        /** Calculate u and v axes from w, which is the surface normal.
         */
        SPICA_EXPORTS void calcLocalCoords(const Vector3D& w, Vector3D* u, Vector3D* v);

        SPICA_EXPORTS bool checkTotalReflection(const bool isIncoming,
                                                const Vector3D& in,
                                                const Vector3D& normal,
                                                const Vector3D& orientNormal,
                                                Vector3D* reflectDir,
                                                Vector3D* refractDir,
                                                double* fresnelRef,
                                                double* fresnelTransmit);

        /** Standard radiance simulator
         *  @param[in] scene: rendered scene
         *  @param[in] params: rendering parameters
         *  @param[in] ray: ray casted from camera
         *  @param[in] rseq: random number sequence
         *  @param[in] bounces: # of bounces
         */
        SPICA_EXPORTS Spectrum radiance(const Scene& scene, const RenderParameters& params,
                                        const Ray& ray, Stack<double>& rands, int bounces);

        SPICA_EXPORTS Spectrum directLight(const Scene& scene,
                                           const Vector3D& pos,
                                           const Vector3D& in,
                                           const Vector3D& normal, 
                                           const BSDF& bsdf,
                                           Stack<double>& rstk);

    }  // namespace helper

}  // namespace spica

#endif  // _SPICA_RENDERER_HELPER_H_
