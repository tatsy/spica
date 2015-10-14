#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MLT_RENDERER_H_
#define _SPICA_MLT_RENDERER_H_

#include "../core/forward_decl.h"
#include "renderer_interface.h"

namespace spica {

    /** Metropolis light transport.
     *  @ingroup renderer_module
     */
    class SPICA_EXPORTS MLTRenderer : private IRenderer {
    public:
        /** Constructor.
         */
        MLTRenderer();

        /** Destructor.
         */
        ~MLTRenderer();

        /** MLT rendering process.
         *  @param scene: The rendering scene
         *  @param camera: The camera viewing the scene
         *  @param params: Rendering parameters
         */
        void render(const Scene& scene, const Camera& camera, const RenderParameters& params) override;
    };

}  // namespace spica

#endif  // SPICA_MLT_RENDERER_H_
