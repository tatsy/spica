#define SPICA_API_EXPORT
#include "integrator.h"

#include "../core/memory.h"
#include "../core/parallel.h"
#include "../core/interaction.h"

#include "../bxdf/bxdf.h"
#include "../bxdf/bsdf.h"
#include "../bxdf/fresnel.h"

#include "../camera/camera.h"
#include "../image/film.h"
#include "../random/sampler.h"
#include "render_parameters.h"

namespace spica {

// -----------------------------------------------------------------------------
// Integrator method definitions
// -----------------------------------------------------------------------------

Integrator::Integrator(const std::shared_ptr<const Camera>& camera)
    : camera_{ camera } {
}

Integrator::~Integrator() {
}

// -----------------------------------------------------------------------------
// SamplerIntegrator method definitions
// -----------------------------------------------------------------------------

SamplerIntegrator::SamplerIntegrator(const std::shared_ptr<const Camera>& camera,
                                     const std::shared_ptr<Sampler>& sampler)
    : Integrator{ camera }
    , sampler_{ sampler } {
}

SamplerIntegrator::~SamplerIntegrator() {
}

void SamplerIntegrator::render(const Scene& scene,
                               const RenderParameters& params) const {
    // Preparation for accouting for BSSRDF
    // _integrator->initialize(scene);

    const int width = camera_->film()->resolution().x();
    const int height = camera_->film()->resolution().y();

    // Prepare samplers and memory arenas
    auto samplers = std::vector<std::unique_ptr<Sampler>>(kNumThreads);

    // Distribute rendering tasks
    const int taskPerThread = (height + kNumThreads - 1) / kNumThreads;
    std::vector<std::vector<int> > tasks(kNumThreads);
    for (int y = 0; y < height; y++) {
        tasks[y % kNumThreads].push_back(y);
    }

    // Trace rays
    for (int i = 0; i < params.samplePerPixel(); i++) {
        if (i % kNumThreads == 0) {
            // _integrator->construct(scene, params);
        }

        if (i % 16 == 0) {
            for (int t = 0; t < kNumThreads; t++) {
                auto seed = static_cast<unsigned int>(time(0) + kNumThreads * i + t);
                samplers[t] = sampler_->clone(seed);
            }
        }

        auto arenas = std::make_unique<MemoryArena[]>(kNumThreads);
        for (int t = 0; t < taskPerThread; t++) {
            // ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
            parallel_for(0, kNumThreads, [&](int threadID) {
                samplers[threadID]->startNextSample();
                if (t < tasks[threadID].size()) {
                    const int y = tasks[threadID][t];
                    for (int x = 0; x < width; x++) {
                        const Point2d randFilm = samplers[threadID]->get2D();
                        const Point2d randLens = samplers[threadID]->get2D();
                        const Ray ray = camera_->spawnRay(Point2i(x, y), randFilm, randLens);

                        const Point2i pixel(width - x - 1, y);
                        camera_->film()->addPixel(pixel, randFilm,
                                                  Li(scene, params, ray, *samplers[threadID],
                                                     arenas[threadID]));
                    }
                }
                arenas[threadID].reset();
            });
        }
        arenas.reset(nullptr);
        camera_->film()->save(i + 1);
    }
    std::cout << "Finish!!" << std::endl;
}

Spectrum SamplerIntegrator::specularReflect(const Scene& scene,
                                            const RenderParameters& params,
                                            const Ray& ray,
                                            const SurfaceInteraction& isect,
                                            Sampler& sampler,
                                            MemoryArena& arena,
                                            int depth) const {
    Vector3d wi, wo = isect.wo();
    double pdf;
    BxDFType type = BxDFType::Reflection | BxDFType::Specular;
    Spectrum f = isect.bsdf()->sample(wo, &wi, sampler.get2D(), &pdf, type);
    
    const Normal3d nrm = isect.normal();
    if (pdf > 0.0 && !f.isBlack() && vect::absDot(wi, nrm) != 0.0) {
        Ray r = isect.spawnRay(wi);
        return f * Li(scene, params, r, sampler, arena, depth + 1) *
               vect::absDot(wi, nrm) / pdf;
    } 
    return Spectrum(0.0);
}

Spectrum SamplerIntegrator::specularTransmit(const Scene& scene,
                                             const RenderParameters& params,
                                             const Ray& ray,
                                             const SurfaceInteraction& isect,
                                             Sampler& sampler,
                                             MemoryArena& arena,
                                             int depth) const {
    Vector3d wi, wo = isect.wo();
    double pdf;
    const Point3d& pos = isect.pos();
    const Normal3d& nrm = isect.normal();
    BxDFType type = BxDFType::Transmission | BxDFType::Specular;
    Spectrum f = isect.bsdf()->sample(wo, &wi, sampler.get2D(), &pdf, type);
    if (pdf > 0.0 && !f.isBlack() && vect::absDot(wi, nrm) != 0.0) {
        Ray r = isect.spawnRay(wi);
        return f * Li(scene, params, r, sampler, arena, depth + 1) *
               vect::absDot(wi, nrm) / pdf;
    }
    return Spectrum(0.0);
}

}  // namespace spica
