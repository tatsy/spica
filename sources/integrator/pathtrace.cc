#define SPICA_API_EXPORT
#include "pathtrace.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "../core/common.h"
#include "../core/memory.h"
#include "../core/spectrum.h"
#include "../core/sampling.h"
#include "../core/interaction.h"

#include "../image/film.h"
#include "../image/image.h"
#include "../image/tmo.h"

#include "../math/vector3d.h"

#include "../random/halton.h"
#include "../random/sampler.h"

#include "../bxdf/bsdf.h"
#include "../bxdf/bxdf.h"
#include "../bxdf/bssrdf.h"

#include "../scenes/scene.h"

#include "integrator.h"
#include "mis.h"

#include "../camera/camera.h"

#include "renderer_helper.h"
#include "render_parameters.h"
// #include "subsurface_integrator.h"

namespace spica {

PathIntegrator::PathIntegrator(std::shared_ptr<Camera>& camera,
                               std::shared_ptr<Sampler>& sampler)
    : Integrator{ camera }
    , sampler_{ sampler } {
}

PathIntegrator::~PathIntegrator() {
}

void PathIntegrator::render(const Scene& scene,
                            const RenderParameters& params) const {
    // Preparation for accouting for BSSRDF
    // _integrator->initialize(scene);

    const int width = camera_->film()->resolution().x();
    const int height = camera_->film()->resolution().y();

    // Prepare samplers and memory arenas
    auto samplers = std::vector<std::unique_ptr<Sampler>>(kNumThreads);
    auto arenas   = std::vector<MemoryArena>(kNumThreads);
    for (int t = 0; t < kNumThreads; t++) {
        auto seed = static_cast<unsigned int>(time(0) + t);
        samplers[t] = sampler_->clone(seed);
    }

    // Distribute rendering tasks
    const int taskPerThread = (height + kNumThreads - 1) / kNumThreads;
    std::vector<std::vector<int> > tasks(kNumThreads);
    for (int y = 0; y < height; y++) {
        tasks[y % kNumThreads].push_back(y);
    }

    // Trace rays
    int processed = 0;
    for (int i = 0; i < params.samplePerPixel(); i++) {
        if (i % kNumThreads == 0) {
            // _integrator->construct(scene, params);
        }

        for (int t = 0; t < taskPerThread; t++) {
            ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
                if (t < tasks[threadID].size()) {
                    const int y = tasks[threadID][t];
                    for (int x = 0; x < width; x++) {
                        const Point2D randFilm = samplers[threadID]->get2D();
                        const Point2D randLens = samplers[threadID]->get2D();
                        const Ray ray = camera_->spawnRay(Point2i(x, y), randFilm, randLens);

                        const Point2i pixel(width - x - 1, y);
                        camera_->film()->
                            addPixel(pixel, randFilm,
                                     Li(scene, params, ray, *samplers[threadID], arenas[threadID]));
                    }
                }
                arenas[threadID].reset();
            }
        }
        camera_->film()->save(i);
    }
    std::cout << "Finish!!" << std::endl;
}

Spectrum PathIntegrator::Li(const Scene& scene,
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
        Vector3D wo = -ray.dir();
        Vector3D wi;
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
