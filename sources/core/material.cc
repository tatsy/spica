#define SPICA_API_EXPORT
#include "material.h"

namespace spica {

SurfaceMaterial::~SurfaceMaterial() {
}

void SurfaceMaterial::bump(SurfaceInteraction *intr, const std::shared_ptr<Texture<double>> &bumpMap) {
}

SubsurfaceMaterial::~SubsurfaceMaterial() {
}

Material::Material(const std::shared_ptr<SurfaceMaterial> &bsdf,
                   const std::shared_ptr<SubsurfaceMaterial> &subsurface)
    : bsdf_{bsdf}
    , subsurface_{subsurface} {
}

void Material::setScatterFuncs(SurfaceInteraction *intr, MemoryArena &arena) const {
    bsdf_->setScatterFuncs(intr, arena);
    if (subsurface_) {
        subsurface_->setScatterFuncs(intr, arena);
    }
}

}  // namespace spica
