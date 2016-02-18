#define SPICA_API_EXPORT
#include "interaction.h"

#include "../core/ray.h"
#include "../core/primitive.h"
#include "../math/vector3d.h"
#include "../bxdf/bsdf.h"
#include "../light/area_light.h"

namespace spica {

// -----------------------------------------------------------------------------
// Interaction method definitions
// -----------------------------------------------------------------------------

Interaction::Interaction()
    : pos_{}
    , normal_{} 
    , wo_{} {
}

Interaction::Interaction(const Point& p, const Normal& n,
                            const Vector3D& wo)
    : pos_{ p }
    , normal_{ n }
    , wo_{ wo } {
}

Interaction::~Interaction() {
}

Interaction::Interaction(const Interaction& intr)
    : Interaction{} {
    this->operator=(intr);
}

Interaction& Interaction::operator=(const Interaction& intr) {
    this->pos_    = intr.pos_;
    this->normal_ = intr.normal_;
    this->wo_     = intr.wo_;
    return *this;
}

Ray Interaction::spawnRay(const Vector3D& wi) const {
    return Ray(pos_, wi);
}

Ray Interaction::spawnRayTo(const Interaction& intr) const {
    Vector3D d = intr.pos_ - pos_;
    return Ray(pos_, d, std::max(0.0, d.norm() - EPS));
}


// -----------------------------------------------------------------------------
// SurfaceInteraction method definitions
// -----------------------------------------------------------------------------

SurfaceInteraction::SurfaceInteraction() 
    : Interaction{}
    , uv_{}
    , dpdu_{}
    , dpdv_{}
    , dndu_{}
    , dndv_{}
    , bsdf_{} {
}

SurfaceInteraction::SurfaceInteraction(const Point& pos, const Point2D& uv,
                                       const Vector3D& wo,
                                       const Vector3D& dpdu, const Vector3D& dpdv,
                                       const Normal3D& dndu, const Normal3D& dndv,
                                       const Shape* shape)
    : Interaction{ pos, Normal(vect::normalize(vect::cross(dpdu, dpdv))), wo }
    , uv_{ uv }
    , dpdu_{ dpdu }
    , dpdv_{ dpdv }
    , dndu_{ dndu }
    , dndv_{ dndv }
    , shape_{ shape } {
}

SurfaceInteraction::SurfaceInteraction(const SurfaceInteraction& intr)
    : SurfaceInteraction{} {
    this->operator=(intr);
}

SurfaceInteraction::~SurfaceInteraction() {
}

SurfaceInteraction& SurfaceInteraction::operator=(const SurfaceInteraction& intr) {
    Interaction::operator=(intr);
    this->uv_ = intr.uv_;
    this->dpdu_ = intr.dpdu_;
    this->dpdv_ = intr.dpdv_;
    this->dndu_ = intr.dndu_;
    this->shape_ = intr.shape_;
    return *this;
}

void SurfaceInteraction::computeDifferentials(const Ray& ray) {
    //if (ray.hasDifferential()) {
    //} else {
    dudx_ = dvdx_ = 0.0;
    dudy_ = dvdy_ = 0.0;
    dpdx_ = dpdy_ = Vector3D(0.0, 0.0, 0.0);
    //}
}

void SurfaceInteraction::setScatterFuncs(const Ray& ray, MemoryArena& arena) {
    computeDifferentials(ray);
    primitive_->setScatterFuncs(this, arena);
}

Spectrum SurfaceInteraction::Le(const Vector3D& w) const {
    const AreaLight* area = primitive_->areaLight();
    return area ? area->L(*this, w) : Spectrum(0.0);
}

}  // namespace spica
