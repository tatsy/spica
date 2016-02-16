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

    Scene::Scene(Scene&& scene)
        : Scene{} {
        this->operator=(std::move(scene));
    }

    Scene::~Scene() {
    }

    Scene& Scene::operator=(Scene&& scene) {
        this->aggregate_  = std::move(scene.aggregate_);
        this->lights_     = std::move(scene.lights_);
        this->worldBound_ = std::move(scene.worldBound_);
        return *this;
    }

    bool Scene::intersect(Ray& ray, SurfaceInteraction* isect) const {
        return aggregate_->intersect(ray, isect);
    }

}  // namespace spica
