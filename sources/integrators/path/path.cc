#define SPICA_API_EXPORT
#include "path.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "core/common.h"
#include "core/memory.h"
#include "core/spectrum.h"
#include "core/renderparams.h"
#include "core/vector3d.h"
#include "core/sampler.h"

#include "core/sampling.h"
#include "core/interaction.h"
#include "core/film.h"

#include "core/bsdf.h"
#include "core/bxdf.h"
#include "core/bssrdf.h"

#include "core/scene.h"

#include "core/integrator.h"
#include "core/mis.h"

namespace spica {

PathIntegrator::PathIntegrator(const std::shared_ptr<Sampler>& sampler)
    : SamplerIntegrator{ sampler }
    , sampler_{ sampler } {
}

PathIntegrator::PathIntegrator(RenderParams &params) 
    : PathIntegrator{std::static_pointer_cast<Sampler>(params.getObject("sampler"))} {
}

PathIntegrator::~PathIntegrator() {
}

Spectrum PathIntegrator::Li(const Scene& scene,
                          RenderParams& params,
                          const Ray& r,
                          Sampler& sampler,
                          MemoryArena& arena,
                          int depth) const {
    Ray ray(r);
    Spectrum L(0.0);
    Spectrum beta(1.0);
    bool specularBounce = false;
    int bounces;
    for (bounces = 0; ; bounces++) {
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        // Sample Le which contributes without any loss
        if (bounces == 0 || specularBounce) {
            if (isIntersect) {
                L += beta * isect.Le(-ray.dir());
            } else {
                for (const auto& light : scene.lights()) {
                    L += beta * light->Le(ray);
                }
            }
        }

        if (!isIntersect || bounces >= params.getInt("maxDepth")) break;

        isect.setScatterFuncs(ray, arena);
        if (!isect.bsdf()) {
            ray = isect.spawnRay(ray.dir());
            bounces--;
            continue;
        }

        if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
            Spectrum Ld = beta * uniformSampleOneLight(isect, scene, arena, sampler);
            L += Ld;
        }

        // Process BxDF
        Vector3d wo = -ray.dir();
        Vector3d wi;
        double pdf;
        BxDFType sampledType;
        Spectrum ref = isect.bsdf()->sample(wo, &wi, sampler.get2D(), &pdf,
                                            BxDFType::All, &sampledType);

        if (ref.isBlack() || pdf == 0.0) break;

        beta *= ref * vect::absDot(wi, isect.ns()) / pdf;
        specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
        ray = isect.spawnRay(wi);

        // Account for BSSRDF
        if (isect.bssrdf() && (sampledType & BxDFType::Transmission) != BxDFType::None) {
            SurfaceInteraction pi;
            Spectrum S = isect.bssrdf()->sample(scene, sampler.get1D(),
                sampler.get2D(), arena, &pi, &pdf);

            if (S.isBlack() || pdf == 0.0) break;
            beta *= S / pdf;

            L += beta * uniformSampleOneLight(pi, scene, arena, sampler);

            Spectrum f = pi.bsdf()->sample(pi.wo(), &wi, sampler.get2D(), &pdf,
                                           BxDFType::All, &sampledType);
            if (f.isBlack() || pdf == 0.0) break;
            beta *= f * vect::absDot(wi, pi.normal()) / pdf;

            specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
            ray = pi.spawnRay(wi);
        }

        // Russian roulette
        if (bounces > 3) {
            double continueProbability = std::min(0.95, beta.gray());
            if (sampler.get1D() > continueProbability) break;
            beta /= continueProbability;
        }
    }

    return L;
}

}  // namespace spica
