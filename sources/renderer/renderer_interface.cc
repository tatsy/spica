#define SPICA_API_EXPORT
#include "renderer_interface.h"

#include "../scenes/scene.h"
#include "render_parameters.h"
// #include "subsurface_integrator.h"

#include "../camera/camera.h"

namespace spica {

    IRenderer::IRenderer(RendererType type)
        : _result{}
        // , _integrator{std::make_unique<SubsurfaceIntegrator>()}
        , _type(type) {
    }

    IRenderer::~IRenderer() {
    }

}  // namespace spica
