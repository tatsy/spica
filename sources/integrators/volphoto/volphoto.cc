#define SPICA_API_EXPORT
#include "volphoto.h"

#include <cmath>
#include <ctime>
#include <cstdio>
#include <atomic>
#include <algorithm>
#include <mutex>

#include "core/parallel.h"
#include "core/memory.h"
#include "core/kdtree.h"
#include "core/interaction.h"
#include "core/sampling.h"
#include "core/renderparams.h"
#include "core/scene.h"

#include "core/bsdf.h"
#include "core/bxdf.h"
#include "core/bssrdf.h"
#include "core/phase.h"
#include "core/mis.h"

#include "integrators/photonmap.h"

namespace spica {

VolPhotoIntegrator::VolPhotoIntegrator(const std::shared_ptr<Sampler>& sampler, double alpha)
    : SamplerIntegrator{ sampler }
    , globalMap_{}
    , causticsMap_{}
    , volumetricMap_{}
    , alpha_{ alpha } {
    globalMap_ = std::make_unique<PhotonMap>(PhotonMapType::Global);
    causticsMap_ = std::make_unique<PhotonMap>(PhotonMapType::Caustics);
    volumetricMap_ = std::make_unique<PhotonMap>(PhotonMapType::Volumetric);
}

VolPhotoIntegrator::VolPhotoIntegrator(RenderParams &params)
        : VolPhotoIntegrator{ std::static_pointer_cast<Sampler>(params.getObject("sampler")),
                              params.getDouble("lookupRadiusRatio", 0.8) } {
}

void VolPhotoIntegrator::initialize(const std::shared_ptr<const Camera> &camera,
                                    const Scene& scene,
                                    RenderParams& params,
                                    Sampler& sampler) {
    // Compute global radius
    Bounds3d bounds = scene.worldBound();
    lookupRadiusScale_ = (bounds.posMax() - bounds.posMin()).norm() * 0.5;
}

void VolPhotoIntegrator::loopStarted(const std::shared_ptr<const Camera> &camera,
                                     const Scene& scene,
                                     RenderParams& params,
                                     Sampler& sampler) {
    // Construct photon map
    globalMap_->construct(scene, params, sampler, params.getInt("globalPhotons", 250000));
    causticsMap_->construct(scene, params, sampler, params.getInt("causticsPhotons", 250000));
    volumetricMap_->construct(scene, params, sampler, params.getInt("volumetricPhotons", 250000));
}

void VolPhotoIntegrator::loopFinished(const std::shared_ptr<const Camera> &camera,
                                      const Scene& scene,
                                      RenderParams& params,
                                      Sampler& sampler) {
    // Scale global radius
    lookupRadiusScale_ = std::max(1.0, lookupRadiusScale_ * alpha_);
}


Spectrum VolPhotoIntegrator::Li(const Scene& scene,
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

    std::mutex mtx;
    for (int bounces = 0; ; bounces++) {
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        // Sample participating media
        MediumInteraction mi;
        if (ray.medium()) beta *= ray.medium()->sample(ray, sampler, arena, &mi);
        if (beta.isBlack()) break;

        // Sample participating media
        if (mi.isValid()) {
            if (bounces >= maxBounces) break;

            L += beta * uniformSampleOneLight(mi, scene, arena, sampler, true);
            const double volumetricLookupRadius = params.getDouble("volumetricLookupRadius", 0.125) * lookupRadiusScale_;
            L += beta * volumetricMap_->evaluateL(mi, gatherPhotons, volumetricLookupRadius);
            break;

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

            Spectrum bnew = beta * ref * vect::absDot(wi, isect.ns()) / pdf;

            if ((sampledType & BxDFType::Diffuse) != BxDFType::None &&
                (sampledType & BxDFType::Reflection) != BxDFType::None) {
                const double globalLookupRadius = params.getDouble("globalLookupRadius", 0.125) * lookupRadiusScale_;
                L += beta * globalMap_->evaluateL(isect, gatherPhotons, globalLookupRadius);
                const double causticsLookupRadius = params.getDouble("causticsLookupRadius", 0.125) * lookupRadiusScale_;
                L += beta * causticsMap_->evaluateL(isect, gatherPhotons, causticsLookupRadius);
                break;

            } else {
                L += Ld;
            }

            ray = isect.spawnRay(wi);
            beta = bnew;
            specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;

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
