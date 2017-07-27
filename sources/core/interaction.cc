#define SPICA_API_EXPORT
#include "interaction.h"

#include "core/float.h"
#include "core/ray.h"
#include "core/vector3d.h"

#include "core/light.h"
#include "core/primitive.h"

namespace spica {

namespace {

Point3d offsetRayOrigin(const Point3d& p, const Normal3d& n, const Vector3d& w) {
    Vector3d offset = Vector3d(n) * 1.0e-3;
    if (vect::dot(w, n) < 0.0) offset = -offset;

    Point3d po = p + offset;
    double ret[3];
    for (int i = 0; i < 3; i++) {
        if (offset[i] > 0.0) {
            ret[i] = nextFloatUp(po[i]);
        } else {
            ret[i] = nextFloatDown(po[i]);
        }
    }
    return Point3d(ret[0], ret[1], ret[2]);
}

}  // anonymous namespace

// -----------------------------------------------------------------------------
// Interaction method definitions
// -----------------------------------------------------------------------------

Interaction::Interaction()
    : pos_{}
    , normal_{} 
    , wo_{}
    , mediumInterface_{} {
}

Interaction::Interaction(const Point3d& p, const Normal3d& n,
                         const Vector3d& wo)
    : pos_{ p }
    , normal_{ n }
    , wo_{ wo }
    , mediumInterface_{} {
}

Interaction::Interaction(const Point3d& p, const Vector3d& wo,
                         const MediumInterface& mediumInterface)
    : pos_{ p }
    , normal_{}
    , wo_{ wo }
    , mediumInterface_{ mediumInterface } {
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
    this->mediumInterface_ = intr.mediumInterface_;
    return *this;
}

Ray Interaction::spawnRay(const Vector3d& wi) const {
    Point3d origin = offsetRayOrigin(pos_, normal_, wi);
    return Ray(origin, wi, INFTY); 
}

Ray Interaction::spawnRayTo(const Point3d& p) const {
    Vector3d d = p - pos_;
    Point3d origin = offsetRayOrigin(pos_, normal_, d);
    return Ray(origin, d, d.norm());
}

Ray Interaction::spawnRayTo(const Interaction& intr) const {
    Point3d origin = offsetRayOrigin(pos_, normal_, intr.pos_ - pos_);
    Point3d target = offsetRayOrigin(intr.pos_, intr.normal_, origin - intr.pos_);
    Vector3d d = target - origin;
    return Ray(origin, d, d.norm());
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

SurfaceInteraction::SurfaceInteraction(const Point3d& pos, const Point2d& uv,
                                       const Vector3d& wo,
                                       const Vector3d& dpdu, const Vector3d& dpdv,
                                       const Normal3d& dndu, const Normal3d& dndv,
                                       const Shape* shape)
    : Interaction{ pos, Normal3d(vect::normalize(vect::cross(dpdu, dpdv))), wo }
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
    this->primitive_ = intr.primitive_;
    this->bsdf_ = intr.bsdf_;
    this->bssrdf_ = intr.bssrdf_;
    return *this;
}

void SurfaceInteraction::computeDifferentials(const Ray& ray) {
    //if (ray.hasDifferential()) {
    //} else {
    dudx_ = dvdx_ = 0.0;
    dudy_ = dvdy_ = 0.0;
    dpdx_ = dpdy_ = Vector3d(0.0, 0.0, 0.0);
    //}
}

void SurfaceInteraction::setScatterFuncs(const Ray& ray, MemoryArena& arena) {
    computeDifferentials(ray);
    primitive_->setScatterFuncs(this, arena);
}

Spectrum SurfaceInteraction::Le(const Vector3d& w) const {
    const Light* area = primitive_->light();
    return area->Larea(*this, w);
}

// -----------------------------------------------------------------------------
// SurfaceInteraction method definitions
// -----------------------------------------------------------------------------

MediumInteraction::MediumInteraction()
    : phase_{ nullptr } {
}

MediumInteraction::MediumInteraction(const Point3d& p, const Vector3d& wo,
                                     const Medium* medium,
                                     const PhaseFunction* phase)
    : Interaction{ p, wo,  MediumInterface(medium) }
    , phase_{ phase } {
}

bool MediumInteraction::isValid() const {
    return phase_ != nullptr;
}

}  // namespace spica
