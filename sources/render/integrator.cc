#define SPICA_API_EXPORT
#include "integrator.h"

#include "core/memory.h"
#include "core/parallel.h"
#include "core/renderparams.h"
#include "core/sampler.h"

#include "render/interaction.h"
#include "render/bxdf.h"
#include "render/bsdf.h"
#include "render/fresnel.h"

#include "render/camera.h"
#include "render/film.h"

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

void SamplerIntegrator::initialize(const Scene& scene,
                                   const RenderParams& parmas,
                                   Sampler& sampler) {
}

void SamplerIntegrator::loopStarted(const Scene& scene,
                                    const RenderParams& parmas,
                                    Sampler& sampler) {
}

void SamplerIntegrator::loopFinished(const Scene& scene,
                                     const RenderParams& parmas,
                                     Sampler& sampler) {
}

void SamplerIntegrator::render(const Scene& scene,
                               const RenderParams& params) {
    // Initialization
    auto initSampler = sampler_->clone((unsigned int)time(0));
    initialize(scene, params, *initSampler);

    const int width = camera_->film()->resolution().x();
    const int height = camera_->film()->resolution().y();

    const int numThreads = numSystemThreads();
    auto samplers = std::vector<std::unique_ptr<Sampler>>(numThreads);
    auto arenas   = std::vector<MemoryArena>(numThreads);

    // Trace rays
    const int numPixels  = width * height;
    const int numSamples = params.get<int>("NUM_SAMPLES");
    for (int i = 0; i < numSamples; i++) {
        // Before loop computations
        loopStarted(scene, params, *initSampler);

        // Prepare samplers
        if (i % numThreads == 0) {
            for (int t = 0; t < numThreads; t++) {
                auto seed = static_cast<unsigned int>(time(0) + t);
                samplers[t] = sampler_->clone(seed);
            }
        }

        std::atomic<int> proc(0);
        parallel_for(0, numPixels, [&](int pid) {
            const int threadID = getThreadID();
            const int y = pid / width;
            const int x = pid % width;
            const Point2d randFilm = samplers[threadID]->get2D();
            const Point2d randLens = samplers[threadID]->get2D();
            const Ray ray = camera_->spawnRay(Point2i(x, y), randFilm, randLens);

            const Point2i pixel(width - x - 1, y);
            camera_->film()->addPixel(pixel, randFilm,
                                        Li(scene, params, ray, *samplers[threadID],
                                            arenas[threadID]));

            proc++;
            if (proc % 1000 == 0) {
                printf("%6.2f %% processed...\r", 100.0 * proc / numPixels);
                fflush(stdout);
            }
        });

        camera_->film()->save(i + 1);

        for (int t = 0; t < numThreads; t++) {
            arenas[t].reset();
        }

        // After loop computations
        loopFinished(scene, params, *initSampler);
    }
    std::cout << "Finish!!" << std::endl;
}

Spectrum SamplerIntegrator::specularReflect(const Scene& scene,
                                            const RenderParams& params,
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
                                             const RenderParams& params,
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
