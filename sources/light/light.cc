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

    LightType Light::type() const {
        return type_;
    }

}  // namespace spica
