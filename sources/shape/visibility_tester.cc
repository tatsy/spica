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

    Spectrum VisibilityTester::transmittance(const Scene& scene, Sampler& sampler) const {
        Ray ray(p1_.spawnRayTo(p2_));
        Spectrum tr(1.0);
        for (;;) {
            SurfaceInteraction isect;
            bool hitSurface = scene.intersect(ray, &isect);
            if (hitSurface && isect.primitive()->material() != nullptr) {
                return Spectrum(0.0);
            }

            if (ray.medium()) tr *= ray.medium()->Tr(ray, sampler);

            if (!hitSurface) break;
            ray = isect.spawnRayTo(p2_);
        }
        return tr;
    }

}  // namespace spica

