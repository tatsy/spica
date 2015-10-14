#ifndef SPICA_BPT_RENDERER_H_
#define SPICA_BPT_RENDERER_H_

#include "../core/forward_decl.h"
#include "renderer_interface.h"

namespace spica {
    
    /** Bidirectional path tracing
     *  @ingroup renderer_module
     */
    class SPICA_EXPORTS BDPTRenderer : public IRenderer {
    public:
        /** BDPT renderer constructor.
         */
        BDPTRenderer();

        /** BDPT renderer destructor.
         */
        ~BDPTRenderer();

        /** Rendering process.
         */
        void render(const Scene& scene, const Camera& camera,
                    const RenderParameters& params) override;

    private:
        /** Redering a pixel.
         */
        void renderPixel(const Scene& scene, const DoFCamera& camera,
                         const RenderParameters& params,
                         RandomSampler& sampler,
                         Image& buffer, int x, int y) const;
    };

}  // namespace spica

#endif // SPICA_BPT_RENDERER_H_
