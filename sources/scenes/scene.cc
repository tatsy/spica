#define SPICA_API_EXPORT
#include "scene.h"

#include "../core/interaction.h"
#include "../medium/medium.h"

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
        bool ist = aggregate_->intersect(ray, isect);
        return ist;
    }

    bool Scene::intersect(Ray& ray) const {
        return aggregate_->intersect(ray);
    }

    bool Scene::intersectTr(Ray& ray, Sampler& sampler,
                            SurfaceInteraction* isect, Spectrum* tr) const {
        *tr = Spectrum(1.0);
        for (;;) {
            bool hitSurface = intersect(ray, isect);
            if (ray.medium()) {
                *tr *= ray.medium()->Tr(ray, sampler);
            }

            if (!hitSurface) return false;
            if (isect->primitive()->material() != nullptr) return true;
            ray = isect->spawnRay(ray.dir());
        }
    }

}  // namespace spica
