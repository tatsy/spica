#define SPICA_API_EXPORT
#include "interaction.h"

#include "../renderer/ray.h"
#include "../math/vector3d.h"

namespace spica {

Interaction::Interaction()
    : pos_{}
    , normal_{} 
    , dir_{} {
}

Interaction::Interaction(const Point& p, const Normal& n,
                            const Vector3D& dir)
    : pos_{ p }
    , normal_{ n }
    , dir_{ dir } {
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
    this->dir_    = intr.dir_;
    return *this;
}

Ray Interaction::nextRay(const Vector3D& wi) const {
    return Ray(pos_, wi);        
}

SurfaceInteraction::SurfaceInteraction(const Point& pos, const Normal& nrm,
                                       const Vector3D& wo, const Point2D& uv)
    : Interaction{ pos, nrm, wo }
    , uv_{ uv } {
}

}  // namespace spica
