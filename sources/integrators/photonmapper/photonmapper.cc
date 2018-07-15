#define SPICA_API_EXPORT
#include "photonmapper.h"

#include <cmath>
#include <ctime>
#include <cstdio>
#include <algorithm>

#include "core/interaction.h"
#include "core/sampling.h"
#include "core/scene.h"

#include "core/bsdf.h"
#include "core/bxdf.h"
#include "core/bssrdf.h"
#include "core/phase.h"
#include "core/mis.h"

namespace spica {

PhotonMapperIntegrator::PhotonMapperIntegrator(const std::shared_ptr<Sampler>& sampler, double alpha)
    : SamplerIntegrator{ sampler }
    , photonmap_{}
    , globalRadius_{}
    , alpha_{ alpha } {
}

PhotonMapperIntegrator::PhotonMapperIntegrator(RenderParams &params)
    : PhotonMapperIntegrator{ std::static_pointer_cast<Sampler>(params.getObject("sampler")),
                              params.getDouble("lookupRadiusRatio", 0.8) } {
}

void PhotonMapperIntegrator::initialize(const std::shared_ptr<const Camera> &camera,
                                        const Scene& scene,
                                        RenderParams& params,
                                        Sampler& sampler) {
    // Compute global radius
    const Bounds3d bounds = scene.worldBound();
    globalRadius_ = (bounds.posMax() - bounds.posMin()).norm() * 0.5;
}

void PhotonMapperIntegrator::loopStarted(const std::shared_ptr<const Camera> &camera,
                                         const Scene& scene,
                                         RenderParams& params,
                                         Sampler& sampler) {
    // Construct photon map
    photonmap_.construct(scene, params, sampler);
}

void PhotonMapperIntegrator::loopFinished(const std::shared_ptr<const Camera> &camera,
                                          const Scene& scene,
                                          RenderParams& params,
                                          Sampler& sampler) {
    // Scale global radius
    globalRadius_ *= alpha_;
}


Spectrum PhotonMapperIntegrator::Li(const Scene& scene,
                                    RenderParams& params,
                                    const Ray& r,
                                    Sampler& sampler,
                                    MemoryArena& arena,
                                    int depth) const {
    Ray ray(r);
    Spectrum L(0.0);
    Spectrum beta(1.0);
    bool specularBounce = false;
    const int maxBounces      = params.getInt("maxDepth", 8);
    const int gatherPhotons   = params.getInt("lookupSize", 32);
    const double gatherRadius = params.getDouble("globalLookupRadius", 8.0);
    for (int bounces = 0; ; bounces++) {
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        // Sample participating media
        MediumInteraction mi;
        if (ray.medium()) beta *= ray.medium()->sample(ray, sampler, arena, &mi);
        if (beta.isBlack()) break;

        if (mi.isValid()) {
            L += beta * uniformSampleOneLight(mi, scene, arena, sampler, true);

            if (bounces >= maxBounces) break;

            Vector3d wo = -ray.dir();
            Vector3d wi;
            mi.phase()->sample(wo, &wi, sampler.get2D());
            ray = mi.spawnRay(wi);
        } else {
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

            if (!isIntersect || bounces >= maxBounces) break;

            isect.setScatterFuncs(ray, arena);
            if (!isect.bsdf()) {
                ray = isect.spawnRay(ray.dir());
                bounces--;
                continue;
            }

            Spectrum Ld(0.0);
            if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
                Ld = beta * uniformSampleOneLight(isect, scene, arena, sampler);
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
                L += beta * photonmap_.evaluateL(isect, gatherPhotons, 
                                                 gatherRadius);
                break;
            } else {
                L += Ld;
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
    }
    return L;
}

}  // namespace spica

