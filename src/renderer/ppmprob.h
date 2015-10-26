#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PHOTON_MAPPING_H_
#define _SPICA_PHOTON_MAPPING_H_

#include "../core/forward_decl.h"

#include "renderer_interface.h"
#include "photon_map.h"

namespace spica {

    /** Progressive photon mapping: a probabilistic approach
     *  @ingroup renderer_module
     */
    class SPICA_EXPORTS PPMPRenderer : public IRenderer {

    private:
        PhotonMap photonMap;
        double    globalRadius;
        static const double kAlpha;

    public:
        /** Constructor.
         */
        PPMPRenderer();

        /** Destructor.
         */
        ~PPMPRenderer();

        /** Rendering process.
         */
        void render(const Scene& scne, const Camera& camera, 
                    const RenderParameters& params) override;

    private:
        Color tracePath(const Scene& scene, const Camera& camera,
                        const RenderParameters& params, Stack<double>& rstk, 
                        int pixelX, int pixelY) const;

        Color radiance(const Scene& scene, const RenderParameters& params, 
                       const Ray& ray, Stack<double>& rstk, int bounces) const;
    };

}  // namespace spica

#endif  // _SPICA_PHOTON_MAPPING_H_
