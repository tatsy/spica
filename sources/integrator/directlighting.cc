#define SPICA_API_EXPORT
#include "directlighting.h"

#include "../core/interaction.h"
#include "../scenes/scene.h"

#include "mis.h"
#include "render_parameters.h"

namespace spica {

DirectLightingIntegrator::DirectLightingIntegrator(
    const std::shared_ptr<const Camera>& camera,
    const std::shared_ptr<Sampler>& sampler)
    : SamplerIntegrator{ camera, sampler } {
}

Spectrum DirectLightingIntegrator::Li(const Scene& scene,
                                      const RenderParameters& params,
                                      const Ray& r,
                                      Sampler& sampler,
                                      MemoryArena& arena,
                                      int depth) const {
    Ray ray(r);
    Spectrum L(0.0);
    SurfaceInteraction isect;
    if (!scene.intersect(ray, &isect)) {
        for (const auto& light : scene.lights()) {
            L += light->Le(ray);
        }
        return L;
    }

    isect.setScatterFuncs(ray, arena);
    if (!isect.bsdf()) {
        Ray nextRay = isect.spawnRay(ray.dir());
        return Li(scene, params, nextRay, sampler, arena, depth);
    }

    if (depth == 0) {
        L += isect.Le(-ray.dir());
    }

    Vector3d wo = isect.wo();
    if (scene.lights().size() > 0) {
        L += mis::uniformSampleOneLight(isect, scene, arena, sampler);
    }

    if (depth + 1 < params.bounceLimit()) {
        L += specularReflect(scene, params, ray, isect, sampler, arena, depth + 1);
        L += specularTransmit(scene, params, ray, isect, sampler, arena, depth + 1);
    }
    return L;
}

}  // namespace spica
