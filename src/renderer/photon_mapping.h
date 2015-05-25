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
#include "../utils/vector3.h"
#include "../utils/axis_comparable.h"
#include "../utils/kdtree.h"
#include "../renderer/material.h"
#include "../renderer/scene.h"
#include "../renderer/camera.h"

#include "photon_map.h"

namespace spica {

    class SPICA_PHOTON_MAPPING_DLL PMRenderer : public Uncopyable {
    private:
        PhotonMap photonMap;

    public:
        PMRenderer();
        ~PMRenderer();

        int render(const Scene& scne, const Camera& camera, const Random& rng, const int samplePerPixel, const int numTargetPhotons, const double targetRadius);

        void buildPM(const Scene& scene, const Camera& camera, const Random& rng, const int numPhotons);

        void savePM(const std::string& filename) const;
        void loadPM(const std::string& filename);

    private:
        Color executePT(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, const Random& rng, const int numTargetPhotons, const double targetRadius) const;
        Color radiance(const Scene& scene, const Ray& ray, const Random& rng, const int numTargetPhotons, const double targetRadius, const int depth, const int depthLimit = 64, const int maxDepth = 5) const;
    };

}

#endif  // _SPICA_PHOTON_MAPPING_H_
