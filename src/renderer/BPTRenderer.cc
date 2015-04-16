#define SPICA_BPT_RENDERER_EXPORT
#include "BPTRenderer.h"

namespace spica {
    
    BPTRenderer::BPTRenderer(int width, int height, int samples, int supsamples)
        : RendererBase(width, height, samples, supsamples)
    {
    }

    BPTRenderer::BPTRenderer(const BPTRenderer& renderer)
        : RendererBase(renderer)
    {
    }

    BPTRenderer::~BPTRenderer()
    {
    }

    BPTRenderer& BPTRenderer::operator=(const BPTRenderer& renderer) {
        RendererBase::operator=(renderer);
        return *this;
    }

    int BPTRenderer::render(const Scene& scene) {
        return 1;
    }
}
