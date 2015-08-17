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
#include "photon_map.h"

namespace spica {

    // --------------------------------------------------
    // Parameter set for photon mapping
    // --------------------------------------------------
    struct PMParams {
        int numPhotons;
        int gatherPhotons;
        int gatherRadius;
        explicit PMParams(const int numPhotons_ = 1000000, const int gatherPhotons_ = 100, const int gatherRadius_ = 20.0)
            : numPhotons(numPhotons_)
            , gatherPhotons(gatherPhotons_)
            , gatherRadius(gatherRadius_)
        {
        }
    };

    class SPICA_PHOTON_MAPPING_DLL PMRenderer : public Uncopyable {
    private:
        PhotonMap photonMap;

    public:
        PMRenderer();
        ~PMRenderer();

        void render(const Scene& scne, const Camera& camera, const int samplePerPixel, const PMParams& params, const RandomType randType = PSEUDO_RANDOM_TWISTER);

    private:
        void buildPM(const Scene& scene, const Camera& camera, const int numPhotons, const RandomType randType);

        Color executePathTracing(const Scene& scene, const Camera& camera, RandomSeq& rseq, const double pixelX, const double pixelY, const int numTargetPhotons, const double targetRadius) const;
        Color radiance(const Scene& scene, const Ray& ray, RandomSeq& rseq, const int numTargetPhotons, const double targetRadius, const int depth, const int depthLimit = 32, const int maxDepth = 6) const;
    };

}

#endif  // _SPICA_PHOTON_MAPPING_H_
