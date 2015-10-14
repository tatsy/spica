#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDERER_HELPER_H_
#define _SPICA_RENDERER_HELPER_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

namespace spica {

    namespace helper {

        /** Calculate u and v axes from w, which is the surface normal.
         */
        void SPICA_EXPORTS calcLocalCoords(const Vector3D& w, Vector3D* u, Vector3D* v);

        bool SPICA_EXPORTS checkTotalReflection(const bool isIncoming,
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
        Color SPICA_EXPORTS radiance(const Scene& scene, const RenderParameters& params,
                                     const Ray& ray, Stack<double>& rands, int bounces);

        Color SPICA_EXPORTS directLight(const Scene& scene,
                                        const Vector3D& pos,
                                        const Vector3D& in,
                                        const Vector3D& normal, 
                                        const BSDF& bsdf,
                                        Stack<double>& rstk);

    }  // namespace helper

}  // namespace spica

#endif  // _SPICA_RENDERER_HELPER_H_
