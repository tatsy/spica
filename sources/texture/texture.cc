#define SPICA_API_EXPORT
#include "texture.h"

#include "../core/point2d.h"
#include "../math/vector2d.h"
#include "../core/interaction.h"

namespace spica {

UVMapping2D::UVMapping2D(double su, double sv, double du, double dv)
    : su_{ su }
    , sv_{ sv }
    , du_{ du }
    , dv_{ dv } {
}

Point2d UVMapping2D::map(const SurfaceInteraction& intr, 
                            Vector2d* dstdx, Vector2d* dstdy) const {
    if (dstdx) *dstdx = Vector2d(su_ * intr.dudx(), sv_ * intr.dvdx());
    if (dstdy) *dstdy = Vector2d(su_ * intr.dudy(), sv_ * intr.dvdy());
    return Point2d(su_ * intr.uv()[0] + du_, sv_ * intr.uv()[1] + dv_);
}

PlanarMapping2D::PlanarMapping2D(const Vector3d& vs, const Vector3d& vt,
                                 double ds, double dt) 
    : vs_{ vs }
    , vt_{ vt }
    , ds_{ ds }
    , dt_{ dt } {
}

Point2d PlanarMapping2D::map(const SurfaceInteraction& intr,
                             Vector2d* dstdx,
                             Vector2d* dstdy) const {
    Vector3d vec(intr.pos());
    return Point2d(ds_ + vect::dot(vec, vs_), dt_ + vect::dot(vec, vt_));
}

}  // namespace spica
