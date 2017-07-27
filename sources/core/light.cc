#define SPICA_API_EXPORT
#include "light.h"

namespace spica {

Light::Light(LightType type, const Transform& lightToWorld, int numSamples)
    : type_{ type }
    , numSamples_{ numSamples } 
    , lightToWorld_{ lightToWorld }
    , worldToLight_{ lightToWorld.inverted() } {
}

Light::~Light() {
}

Spectrum Light::Larea(const Interaction& pLight, const Vector3d& dir) const {
    return Spectrum(0.0);
}

Spectrum Light::Le(const Ray& ray) const {
    return Spectrum(0.0);
}

LightType Light::type() const {
    return type_;
}

}  // namespace spica
