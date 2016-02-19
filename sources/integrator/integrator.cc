#define SPICA_API_EXPORT
#include "integrator.h"

namespace spica {

Integrator::Integrator(std::shared_ptr<Camera>& camera)
    : camera_{ camera } {
}

Integrator::~Integrator() {
}

}  // namespace spica
