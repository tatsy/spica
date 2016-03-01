#define SPICA_API_EXPORT
#include "volpath.h"

#include "../core/ray.h"
#include "../core/interaction.h"

#include "../scenes/scene.h"

#include "../bxdf/bsdf.h"
#include "../bxdf/bxdf.h"
#include "../bxdf/bssrdf.h"
#include "../bxdf/phase.h"

#include "../random/sampler.h"

#include "mis.h"
#include "render_parameters.h"

namespace spica {

VolPathIntegrator::VolPathIntegrator(std::shared_ptr<Camera>& camera,
                                     std::shared_ptr<Sampler>& sampler)
    : PathIntegrator{ camera, sampler } {
}

Spectrum VolPathIntegrator::Li(const Scene& scene,
                               const RenderParameters& params,
                               const Ray& r, Sampler& sampler,
                               MemoryArena& arena, int depth) const {
    Ray ray(r);
    Spectrum L(0.0);
    Spectrum beta(1.0);
    bool specularBounce = false;
    int bounces;
    for (bounces = 0; ; bounces++) {
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        // Sample participating media
        MediumInteraction mi;
        if (ray.medium()) beta *= ray.medium()->sample(ray, sampler, arena, &mi);
        if (beta.isBlack()) break;

        if (mi.isValid()) {
            L += beta * mis::uniformSampleOneLight(mi, scene, arena, sampler, true);

            if (bounces >= params.bounceLimit()) break;

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

                //std::cout << "Hoge: " << (S / pdf) << std::endl; 
                //std::cout << "S: " << S << std::endl;
                //printf("pdf = %f\n", pdf);

                L += beta * mis::uniformSampleOneLight(pi, scene, arena, sampler);

                Spectrum f = pi.bsdf()->sample(pi.wo(), &wi, sampler.get2D(), &pdf,
                                               BxDFType::All, &sampledType);
                if (f.isBlack() || pdf == 0.0) break;
                beta *= f * vect::absDot(wi, pi.normal()) / pdf;

                // auto hoge = f * vect::absDot(wi, pi.normal()) / pdf;
                // std::cout << "hoge: " << hoge << std::endl;

                specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
                ray = pi.spawnRay(wi);
            }
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
