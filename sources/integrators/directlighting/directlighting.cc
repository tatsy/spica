#define SPICA_API_EXPORT
#include "directlighting.h"

#include "core/interaction.h"
#include "core/renderparams.h"
#include "core/scene.h"
#include "core/mis.h"
#include "core/sampler.h"

namespace spica {

DirectLightingIntegrator::DirectLightingIntegrator(
    const std::shared_ptr<Sampler>& sampler)
    : SamplerIntegrator{ sampler } {
}

DirectLightingIntegrator::DirectLightingIntegrator(RenderParams &params)
    : DirectLightingIntegrator{std::static_pointer_cast<Sampler>(params.getObject("sampler", true))} {
}

Spectrum DirectLightingIntegrator::Li(const Scene& scene,
                                      RenderParams& params,
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
        L += uniformSampleOneLight(isect, scene, arena, sampler);
    }

    if (depth + 1 < params.getInt("maxDepth")) {
        L += specularReflect(scene, params, ray, isect, sampler, arena, depth + 1);
        L += specularTransmit(scene, params, ray, isect, sampler, arena, depth + 1);
    }
    return L;
}

}  // namespace spica
