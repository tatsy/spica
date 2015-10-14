#define SPICA_API_EXPORT
#include "renderer_interface.h"

#include "../scenes/scene.h"
#include "render_parameters.h"
#include "subsurface_integrator.h"

#include "../camera/camera.h"

namespace spica {

    IRenderer::IRenderer()
        : _result()
        , _integrator(new SubsurfaceIntegrator) {
    }

    IRenderer::~IRenderer() {
        delete _integrator;
    }

}  // namespace spica
