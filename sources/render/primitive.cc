#define SPICA_API_EXPORT
#include "primitive.h"

#include "core/triangle.h"
#include "render/interaction.h"
#include "render/shape.h"
#include "render/material.h"

namespace spica {

// -----------------------------------------------------------------------------
// Primitive method definitions
// -----------------------------------------------------------------------------

const Light* Aggregate::light() const {
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
                                       const std::shared_ptr<Light>& areaLight,
                                       const std::shared_ptr<MediumInterface>& mediumInterface)
    : Primitive{}
    , shape_{ shape }
    , material_{ material }
    , areaLight_{ areaLight }
    , mediumInterface_{ mediumInterface } {
}

Bounds3d GeometricPrimitive::worldBound() const {
    return shape_->worldBound();
}

bool GeometricPrimitive::intersect(Ray& ray, SurfaceInteraction* isect) const {
    double tHit;
    if (!shape_->intersect(ray, &tHit, isect)) return false;
    ray.setMaxDist(tHit);
    isect->setPrimitive(this);

    if (mediumInterface_ && mediumInterface_->isMediumTransition()) {
        isect->setMediumInterface(*mediumInterface_);
    } else {
        // TODO: Should be reverted?
        //isect->setMediumInterface(MediumInterface(ray.medium()));
    }

    return true;
}

bool GeometricPrimitive::intersect(Ray& ray) const {
    return shape_->intersect(ray);
}

const Material* GeometricPrimitive::material() const {
    return material_.get();
}

const Light* GeometricPrimitive::light() const {
    return areaLight_.get();
}

std::vector<Triangle> GeometricPrimitive::triangulate() const {
    return std::move(shape_->triangulate());
}

void GeometricPrimitive::setScatterFuncs(SurfaceInteraction* isect, MemoryArena& arena) const {
    if (material_) {
        material_->setScatterFuncs(isect, arena);
    }
}

}  // namespace spica