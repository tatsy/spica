#define SPICA_API_EXPORT
#include "pssmlt.h"

#include <atomic>
#include <mutex>
#include <numeric>

#include "core/memory.h"
#include "core/parallel.h"
#include "core/renderparams.h"
#include "core/interaction.h"
#include "core/sampling.h"
#include "core/scene.h"
#include "core/camera.h"
#include "core/film.h"
#include "core/sampler.h"
#include "core/random.h"
#include "core/bsdf.h"
#include "core/bssrdf.h"
#include "core/mis.h"

#include "integrators/primarysample.h"

namespace spica {

PSSMLTIntegrator::PSSMLTIntegrator()
    : Integrator{ } {
}

PSSMLTIntegrator::PSSMLTIntegrator(spica::RenderParams &params)
    : PSSMLTIntegrator{ } {
}

void PSSMLTIntegrator::render(const std::shared_ptr<const Camera>& camera, const Scene& scene, RenderParams& params) {
    // Take parameters.
    const int width  = camera->film()->resolution().x();
    const int height = camera->film()->resolution().y();
    const int numPixels = width * height;
    const double pLarge  = params.getDouble("pLarge", 0.3);
    const int sampleCount = params.getInt("sampleCount", 64);
    const int nBootstrap = params.getInt("luminanceSamples", 500000);
    const int nThreads = numSystemThreads();
    const int nMutate = (numPixels + nThreads - 1) / nThreads;

    const int globalSeed = (uint32_t)time(0);
    std::vector<MemoryArena> arenas(nThreads);

    // Bootstrap
    std::vector<double> sampleWeights(nBootstrap, 0.0);
    parallel_for(0, nBootstrap, [&](int i) {
        // Here, "rngIndex" is for using the same RNG in bootstrapping and mutation.
        const int rngIndex = globalSeed + i;
        auto psSampler = std::make_shared<PSSSampler>(rngIndex, pLarge);

        // Generate bootstrap sample.
        const int threadId = getThreadID();
        PathSample ps = generateSample(camera, scene, params, *psSampler, arenas[threadId]);

        const double I = ps.L().gray();
        sampleWeights[i] = I;

        arenas[threadId].reset();
    });
    MsgInfo("Finish bootstrapping.");

    // Distribution of bootstraps
    Distribution1D bootstrap(sampleWeights);
    const double sumI = std::accumulate(sampleWeights.begin(), sampleWeights.end(), 0.0);
    const double b = sumI / nBootstrap;

    // Mutation
    for (int i = 0; i < sampleCount; i++) {
        std::mutex mtx;
        std::atomic<int64_t> nAccept(0);
        std::atomic<int64_t> nTotal(0);
        parallel_for (0, nThreads, [&](int t) {
            MemoryArena &arena = arenas[t];

            // Select chain.
            Random rng(globalSeed + i * nThreads + t);
            const int index = bootstrap.sampleDiscrete(rng.get1D());
            const int rngIndex = globalSeed + index;
            auto psSampler = std::make_shared<PSSSampler>(rngIndex, pLarge);

            // Generate first sample.
            PathSample currentSample = generateSample(camera, scene, params, *psSampler, arena);

            // Mutation.
            for (int m = 0; m < nMutate; m++) {
                // Generate next sample.
                PathSample nextSample = generateSample(camera, scene, params, *psSampler, arena);
                double acceptRatio = 1.0;
                if (!currentSample.L().isBlack()) {
                    acceptRatio = std::min(1.0, nextSample.L().gray() / currentSample.L().gray());
                }

                // Update image.
                mtx.lock();
                {
                    double currentWeight = (1.0 - acceptRatio) /
                                           ((currentSample.L().gray() / b + psSampler->pLarge()));
                    double nextWeight = (acceptRatio + psSampler->largeStep()) /
                                        ((nextSample.L().gray() / b + psSampler->pLarge()));

                    Point2d currentPixel(width - currentSample.p().x(), currentSample.p().y());
                    Point2d nextPixel(width - nextSample.p().x(), nextSample.p().y());
                    camera->film()->addPixel(currentPixel, currentWeight * currentSample.L());
                    camera->film()->addPixel(nextPixel, nextWeight * nextSample.L());
                }
                mtx.unlock();

                // Update sample.
                if (rng.get1D() < acceptRatio) {
                    nAccept++;              
                    currentSample = nextSample;
                    psSampler->accept();
                } else {
                    psSampler->reject();            
                }
                nTotal++;

                if (nTotal % 1000 == 0 || nTotal == (nThreads * nMutate)) {
                    const double ratio = 100.0 * (nTotal + 1) / (nThreads * nMutate);
                    printf("[ %d / %d ] %6.2f %% processed...\r", i + 1, sampleCount, ratio);
                    fflush(stdout);
                }

                // Reset memory arena.
                arena.reset();
            }
        });
        printf("\n");

        // Report accept / reject ratio.
        const double ratio = 100.0 * nAccept / nTotal;
        MsgInfo("Accept ratio = %5.2f %% (%lld / %lld)", ratio, nAccept.load(), nTotal.load());
        fflush(stdout);

        // Save image.
        camera->film()->save(i + 1, 1.0 / (i + 1));
    }
}

PathSample PSSMLTIntegrator::generateSample(const std::shared_ptr<const Camera> &camera,
                                            const Scene& scene,
                                            RenderParams& params,
                                            Sampler& sampler,
                                            MemoryArena& arena) const {
    // Reset sampler.
    sampler.startNextSample();

    // Generate ray.
    const int width  = camera->film()->resolution().x();
    const int height = camera->film()->resolution().y();

    const double fx = std::min(sampler.get1D() * width, (double)(width - EPS));
    const double fy = std::min(sampler.get1D() * height, (double)(height - EPS));
    const int x = std::min((int)fx, width - 1);
    const int y = std::min((int)fy, height - 1);
    const Point2d randLens = sampler.get2D();
    const Point2d randFilm(fx - x, fy - y);
    const Ray ray = camera->spawnRay(Point2i(x, y), randFilm, randLens);

    Spectrum L = Li(scene, params, ray, sampler, arena);
    return PathSample(Point2d(fx, fy), L);
}

Spectrum PSSMLTIntegrator::Li(const Scene& scene,
                              RenderParams& params,
                              const Ray& r,
                              Sampler& sampler,
                              MemoryArena& arena,
                              int depth) const {
    Ray ray(r);
    Spectrum L(0.0);
    Spectrum beta(1.0);
    bool specularBounce = false;
    
    const int maxDepth = params.getInt("maxDepth");
    for (int bounces = 0; bounces < maxDepth; bounces++) {
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

        if (!isIntersect || bounces >= maxDepth) break;

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
            beta *= f * vect::absDot(wi, pi.ns()) / pdf;

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
