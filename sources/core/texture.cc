#define SPICA_API_EXPORT
#include "texture.h"

#include "core/point2d.h"
#include "core/vector2d.h"
#include "core/interaction.h"

namespace spica {

UVMapping2D::UVMapping2D(double su, double sv, double du, double dv, bool invertHorizontal)
    : su_{ su }
    , sv_{ sv }
    , du_{ du }
    , dv_{ dv }
    , invertHorizontal_{invertHorizontal} {
}

Point2d UVMapping2D::map(const Point3d& p) const {
    const double s = su_ * p[0] + du_;
    const double t = sv_ * p[1] + dv_;
    return Point2d(s, invertHorizontal_ ? 1.0 - t : t);
}

Point2d UVMapping2D::map(const SurfaceInteraction& intr, 
                         Vector2d* dstdx, Vector2d* dstdy) const {
    if (dstdx) *dstdx = Vector2d(su_ * intr.dudx(), sv_ * intr.dvdx());
    if (dstdy) *dstdy = Vector2d(su_ * intr.dudy(), sv_ * intr.dvdy());
    const double s = su_ * intr.uv()[0] + du_;
    const double t = sv_ * intr.uv()[1] + dv_;
    return Point2d(s, invertHorizontal_ ? 1.0 - t : t);
}

PlanarMapping2D::PlanarMapping2D(const Vector3d& vs, const Vector3d& vt,
                                 double ds, double dt) 
    : vs_{ vs }
    , vt_{ vt }
    , ds_{ ds }
    , dt_{ dt } {
}

Point2d PlanarMapping2D::map(const Point3d& p) const {
    Vector3d vec(p);
    return Point2d(ds_ + vect::dot(vec, vs_), dt_ + vect::dot(vec, vt_));
}

Point2d PlanarMapping2D::map(const SurfaceInteraction& intr,
                             Vector2d* dstdx,
                             Vector2d* dstdy) const {
    return map(intr.pos());
}

}  // namespace spica
