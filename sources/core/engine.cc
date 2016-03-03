#define SPICA_API_EXPORT
#include "engine.h"

#include "../integrator/spica_integrator.h"

namespace spica {

Engine::Engine()
    : option_{}
    , integrator_{} {
}

Engine::~Engine() {
}

void Engine::init(const Option& option) {
    this->option_ = option;
}

void Engine::start(const std::string& filename) const {
    /*
    Scene scene;
    RenderParams params;
    
    SceneLoader loader;
    loader.load(scene, camera, params);

    // Instanciate integrator
    if (option_.integrator == "path") {
        integrator_.reset(new PathIntegrator(camera, sampler));
    } else if (option_.integrator == "sppm") {
        integrator_.reset(new SPPMIntegrator(camera, sampler));    
    } else {
        FatalError("Unknown integrator \"%s\" is specified",
                   option_.integrator);
    }
    
    integrator_->render(scene, params);
    */
}

void Engine::cleanup() {
}

}  // namespace spica
