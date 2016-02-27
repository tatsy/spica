#define SPICA_API_EXPORT
#include "visibility_tester.h"

#include "../core/interaction.h"
#include "../scenes/scene.h"

namespace spica {

    VisibilityTester::VisibilityTester()
        : p1_{}
        , p2_{} {
    }

    VisibilityTester::VisibilityTester(const Interaction& p1,
                                       const Interaction& p2)
        : p1_{ p1 }
        , p2_{ p2 } {
    }

    bool VisibilityTester::unoccluded(const Scene& scene) const {
        SurfaceInteraction isect;
        Ray ray = p1_.spawnRayTo(p2_);
        return !scene.intersect(ray, &isect);
    }

}  // namespace spica

