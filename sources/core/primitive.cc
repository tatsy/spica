#define SPICA_API_EXPORT
#include "primitive.h"

namespace spica {

const std::shared_ptr<const AreaLight> Aggregate::areaLight() const {
    Warning("Deprecated function!!");
    return nullptr;
}

const std::shared_ptr<const Material> Aggregate::material() const {
    Warning("Deprecated function!!");
    return nullptr;
}

void Aggregate::setScatterFuncs(SurfaceInteraction* intr,
                                MemoryArena& arena) const {
    Warning("Deprecated function!!");    
}

}  // namespace spica