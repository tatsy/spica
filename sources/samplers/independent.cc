#define SPICA_API_EXPORT
#include "independent.h"

namespace spica {

Independent::Independent(uint32_t seed)
    : random_{seed} {
}

Independent::Independent(RenderParams &params)
    : random_{0} {
}

Independent::~Independent() {
}

double Independent::get1D() {
    return random_.get1D();
}

std::unique_ptr<Sampler> Independent::clone(uint32_t seed) const {
    return std::make_unique<Independent>(seed);
}

}  // namespace spica
