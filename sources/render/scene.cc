#define SPICA_API_EXPORT
#include "scene.h"

#include "render/interaction.h"
#include "render/medium.h"

namespace spica {

    Scene::Scene()
        : aggregate_{}
        , lights_{}
        , worldBound_{} {
    }

    Scene::Scene(const std::shared_ptr<Accelerator>& aggregate,
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

    bool Scene::intersect(Ray& ray) const {
        return aggregate_->intersect(ray);
    }

    bool Scene::intersectTr(Ray& ray, Sampler& sampler, Medium *medium,
                            SurfaceInteraction* isect, Spectrum* tr) const {
        *tr = Spectrum(1.0);
        for (;;) {
            bool hitSurface = intersect(ray, isect);
            if (medium) {
                *tr *= medium->Tr(ray, sampler);
            }

            if (!hitSurface) return false;
            if (isect->primitive()->material() != nullptr) return true;
            ray = isect->spawnRay(ray.dir());
        }
    }

}  // namespace spica
