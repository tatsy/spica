#define SPICA_API_EXPORT
#include "scene.h"

namespace spica {

    Scene::Scene()
        : aggregate_{}
        , lights_{}
        , worldBound_{} {
    }

    Scene::Scene(const std::shared_ptr<Primitive>& aggregate,
                 const std::vector<std::shared_ptr<Light> >& lights)
        : aggregate_{ aggregate }
        , lights_{ lights }
        , worldBound_{ aggregate->worldBound() } {
    }

    Scene::~Scene() {
    }

    bool Scene::intersect(const Ray& ray, SurfaceInteraction* isect) const {
        return aggregate_->intersect(ray, isect);
    }

}  // namespace spica
