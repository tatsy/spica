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

    Point2D UVMapping2D::map(const SurfaceInteraction& intr, 
                             Vector2D* dstdx, Vector2D* dstdy) const {
        *dstdx = Vector2D(su_ * intr.dudx(), sv_ * intr.dvdx());
        *dstdy = Vector2D(su_ * intr.dudy(), sv_ * intr.dvdy());
        return Point2D(su_ * intr.uv()[0] + du_, sv_ * intr.uv()[1] + dv_);
    }

}  // namespace spica
