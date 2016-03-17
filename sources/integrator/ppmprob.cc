#define SPICA_API_EXPORT
#include "ppmprob.h"

#include <cmath>
#include <ctime>
#include <cstdio>
#include <algorithm>

#include "mis.h"
#include "render_parameters.h"

#include "../core/interaction.h"
#include "../core/sampling.h"
#include "../scenes/scene.h"

#include "../bxdf/bsdf.h"
#include "../bxdf/bxdf.h"
#include "../bxdf/bssrdf.h"

namespace spica {

PPMProbIntegrator::PPMProbIntegrator(
    const std::shared_ptr<const Camera>& camera,
    const std::shared_ptr<Sampler>& sampler,
    double alpha)
    : SamplerIntegrator{ camera, sampler }
    , photonmap_{}
    , globalRadius_{}
    , alpha_{ alpha } {
}

PPMProbIntegrator::~PPMProbIntegrator() {
}

void PPMProbIntegrator::initialize(const Scene& scene,
                                   const RenderParameters& params,
                                   Sampler& sampler) {
    // Compute global radius
    Bounds3d bounds = scene.worldBound();
    globalRadius_ = (bounds.posMax() - bounds.posMin()).norm() * 0.5;

    // Construct photon map
    photonmap_.construct(scene, params);
}

void PPMProbIntegrator::startNextLoop(const Scene& scene,
                                      const RenderParameters& params,
                                      Sampler& sampler) {
    // Scale global radius
    globalRadius_ *= alpha_;   

    // Construct photon map
    photonmap_.construct(scene, params);
}

Spectrum PPMProbIntegrator::Li(const Scene& scene,
                               const RenderParameters& params,
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

        if (!isIntersect || bounces >= params.bounceLimit()) break;

        isect.setScatterFuncs(ray, arena);
        if (!isect.bsdf()) {
            ray = isect.spawnRay(ray.dir());
            bounces--;
            continue;
        }

        if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
            Spectrum Ld = beta * mis::uniformSampleOneLight(isect, scene, arena, sampler);
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

        if ((sampledType & BxDFType::Diffuse) != BxDFType::None &&
            (sampledType & BxDFType::Reflection) != BxDFType::None) {
            L += beta * photonmap_.evaluateL(isect, params.gatherPhotons(),
                                             params.gatherRadius());
            break;
        }

        beta *= ref * vect::absDot(wi, isect.normal()) / pdf;
        specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
        ray = isect.spawnRay(wi);

        // Account for BSSRDF
        if (isect.bssrdf() && (sampledType & BxDFType::Transmission) != BxDFType::None) {
            SurfaceInteraction pi;
            Spectrum S = isect.bssrdf()->sample(scene, sampler.get1D(),
                sampler.get2D(), arena, &pi, &pdf);

            if (S.isBlack() || pdf == 0.0) break;
            beta *= S / pdf;

            L += beta * mis::uniformSampleOneLight(pi, scene, arena, sampler);

            Spectrum f = pi.bsdf()->sample(pi.wo(), &wi, sampler.get2D(), &pdf,
                                           BxDFType::All, &sampledType);
            if (f.isBlack() || pdf == 0.0) break;
            beta *= f * vect::absDot(wi, pi.normal()) / pdf;

            specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
            ray = pi.spawnRay(wi);
        }

        // Russian roulette
        if (bounces > 3) {
            double continueProbability = std::min(0.95, beta.luminance());
            if (sampler.get1D() > continueProbability) break;
            beta /= continueProbability;
        }
    }
    return L;
}

}  // namespace spica

