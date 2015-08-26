#define SPICA_RENDERER_INTERFACE_EXPORT
#include "renderer_interface.h"

#include "scene.h"
#include "camera.h"
#include "render_parameters.h"
#include "subsurface_integrator.h"

namespace spica {

    IRenderer::IRenderer()
        : _result()
        , _integrator(new SubsurfaceIntegrator) {
    }

    IRenderer::~IRenderer() {
        delete _integrator;
    }

}  // namespace spica
