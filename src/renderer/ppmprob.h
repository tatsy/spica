#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PHOTON_MAPPING_H_
#define _SPICA_PHOTON_MAPPING_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_PHOTON_MAPPING_EXPORT
        #define SPICA_PHOTON_MAPPING_DLL __declspec(dllexport)
    #else
        #define SPICA_PHOTON_MAPPING_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_PHOTON_MAPPING_DLL
#endif

#include "../utils/uncopyable.h"
#include "../utils/vector3d.h"
#include "../utils/axis_comparable.h"
#include "../utils/kdtree.h"

#include "renderer_constants.h"
#include "render_parameters.h"
#include "photon_map.h"

namespace spica {

    class SPICA_PHOTON_MAPPING_DLL PMRenderer : public Uncopyable {
    private:
        PhotonMap photonMap;

    public:
        PMRenderer();
        ~PMRenderer();

        void render(const Scene& scne, const Camera& camera, const RenderParameters& params);

    private:
        void buildPM(const Scene& scene, const Camera& camera, const int numPhotons, const int maxBounces, const RandomType randType);

        Color executePathTracing(const Scene& scene, const Camera& camera, const RenderParameters& params,
                                 Stack<double>& rstk, double pixelX, double pixelY) const;

        Color radiance(const Scene& scene, const RenderParameters& params, const Ray& ray, Stack<double>& rstk, int bounces) const;
    };

}  // namespace spica

#endif  // _SPICA_PHOTON_MAPPING_H_
