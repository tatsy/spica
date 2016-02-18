#define SPICA_API_EXPORT
#include "primitive.h"

#include "../core/interaction.h"
#include "../shape/shape.h"
#include "../material/material.h"

namespace spica {

// -----------------------------------------------------------------------------
// Primitive method definitions
// -----------------------------------------------------------------------------

const AreaLight* Aggregate::areaLight() const {
    Warning("Deprecated function!!");
    return nullptr;
}

const Material* Aggregate::material() const {
    Warning("Deprecated function!!");
    return nullptr;
}

void Aggregate::setScatterFuncs(SurfaceInteraction* intr,
                                MemoryArena& arena) const {
    Warning("Deprecated function!!");    
}

// -----------------------------------------------------------------------------
// GeometricPrimitive method definitions
// -----------------------------------------------------------------------------

GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape>& shape,
                                       const std::shared_ptr<Material>& material,
                                       const std::shared_ptr<AreaLight>& areaLight)
    : Primitive{}
    , shape_{ shape }
    , material_{ material }
    , areaLight_{ areaLight } {
}

Bounds3d GeometricPrimitive::worldBound() const {
    return shape_->worldBound();
}

bool GeometricPrimitive::intersect(Ray& ray, SurfaceInteraction* isect) const {
    double tHit;
    if (!shape_->intersect(ray, &tHit, isect)) return false;
    ray.setMaxDist(tHit);
    isect->setPrimitive(this);
    return true;
}

const Material* GeometricPrimitive::material() const {
    return material_.get();
}

const AreaLight* GeometricPrimitive::areaLight() const {
    return areaLight_.get();
}

void GeometricPrimitive::setScatterFuncs(SurfaceInteraction* isect, MemoryArena& arena) const {
    if (material_) {
        material_->setScatterFuncs(isect, arena);
    }
}

}  // namespace spica