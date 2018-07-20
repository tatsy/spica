#define SPICA_API_EXPORT
#include "mmlt.h"

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
#include "integrators/bidirectional.h"

static constexpr int NUM_SAMPLE_STREAMS = 3;
static constexpr int CAMERA_STREAM = 0;
static constexpr int LIGHT_STREAM = 1;
static constexpr int CONNECT_STREAM = 2;

namespace spica {

MMLTIntegrator::MMLTIntegrator()
    : Integrator{ } {
}

MMLTIntegrator::MMLTIntegrator(spica::RenderParams &params)
    : MMLTIntegrator{ } {
}

void MMLTIntegrator::render(const std::shared_ptr<const Camera>& camera, const Scene& scene, RenderParams& params) {
    // Take parameters.
    const int width  = camera->film()->resolution().x();
    const int height = camera->film()->resolution().y();
    const int numPixels = width * height;
    const int maxDepth = params.getInt("maxDepth", 8);
    const double pLarge  = params.getDouble("pLarge", 0.3);
    const int sampleCount = params.getInt("sampleCount", 64);
    const int nThreads = numSystemThreads();
    const int nMutate = (numPixels + nThreads - 1) / nThreads;

    const int globalSeed = (uint32_t)time(NULL) % 10007;
    std::vector<MemoryArena> arenas(nThreads);

    // Light distribution.
    Distribution1D lightDist = calcLightPowerDistrib(scene);

    // Bootstrap
    const int nBootstrap = (params.getInt("luminanceSamples", 500000) + maxDepth) / (maxDepth + 1);
    std::vector<double> sampleWeights(nBootstrap * (maxDepth + 1), 0.0);
    parallel_for(0, nBootstrap, [&](int i) {
        for (int depth = 0; depth <= maxDepth; depth++) {
            // Here, "rngIndex" is for using the same RNG in bootstrapping and mutation.
            const int rngIndex = i * (maxDepth + 1) + depth;
            auto psSampler = std::make_shared<PSSSampler>(globalSeed + rngIndex, pLarge, NUM_SAMPLE_STREAMS);

            // Generate bootstrap sample.
            const int threadId = getThreadID();
            PathSample ps = generateSample(camera, scene, params, *psSampler, arenas[threadId], lightDist, depth);

            const double I = ps.L().gray();
            sampleWeights[rngIndex] = I;

            arenas[threadId].reset();
        }
    });
    MsgInfo("Finish bootstrapping.");

    // Distribution of bootstraps.
    Distribution1D bootstrap(sampleWeights);
    const double sumI = std::accumulate(sampleWeights.begin(), sampleWeights.end(), 0.0);
    const double b = sumI / nBootstrap;

    // Mutation
    std::mutex mtx;
    for (int i = 0; i < sampleCount; i++) {
        std::atomic<int64_t> nAccept(0);
        std::atomic<int64_t> nTotal(0);
        parallel_for (0, nThreads, [&](int t) {
            MemoryArena &arena = arenas[t];

            // Select chain.
            Random rng(globalSeed + i * nThreads + t);
            const int rngIndex = bootstrap.sampleDiscrete(rng.get1D());
            const int depth = rngIndex % (maxDepth + 1);
            auto psSampler = std::make_shared<PSSSampler>(globalSeed + rngIndex, pLarge, NUM_SAMPLE_STREAMS);

            // Generate first sample.
            PathSample currentSample = generateSample(camera, scene, params, *psSampler, arena, lightDist, depth);

            // Mutation.
            for (int m = 0; m < nMutate; m++) {
                // Generate next sample.
                PathSample nextSample = generateSample(camera, scene, params, *psSampler, arena, lightDist, depth);
                double acceptRatio = 1.0;
                if (!currentSample.L().isBlack()) {
                    acceptRatio = std::min(1.0, nextSample.L().gray() / currentSample.L().gray());
                }

                // Update image.
                mtx.lock();
                {
                    if (!currentSample.L().isBlack()) {
                        double currentWeight = (1.0 - acceptRatio) /
                                               ((currentSample.L().gray() / b + psSampler->pLarge()));

                        Point2d currentPixel(width - currentSample.p().x(), currentSample.p().y());
                        camera->film()->addPixel(currentPixel, currentWeight * currentSample.L());
                    }

                    if (!nextSample.L().isBlack()) {
                        double nextWeight = (acceptRatio + psSampler->largeStep()) /
                                            ((nextSample.L().gray() / b + psSampler->pLarge()));
                        Point2d nextPixel(width - nextSample.p().x(), nextSample.p().y());
                        camera->film()->addPixel(nextPixel, nextWeight * nextSample.L());
                    }
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

                if (nTotal % 1000 == 0 || nTotal == nThreads * nMutate) {
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

PathSample MMLTIntegrator::generateSample(const std::shared_ptr<const Camera> &camera,
                                          const Scene& scene,
                                          RenderParams& params,
                                          PSSSampler& sampler,
                                          MemoryArena& arena,
                                          const Distribution1D &lightDist,
                                          int depth) const {
    // Start next sample.
    sampler.startNextSample();

    // Compute contribution
    Point2d pixel;
    Spectrum L = Li(camera, scene, sampler, arena, depth, lightDist, &pixel);

    return PathSample(pixel, L);
}

Spectrum MMLTIntegrator::Li(const std::shared_ptr<const Camera> &camera,
                            const Scene& scene, PSSSampler& sampler, MemoryArena& arena,
                            int depth, const Distribution1D &lightDist,
                            Point2d *pixel) const {

    sampler.startStream(CAMERA_STREAM);

    // Determine # of available strategies.
    int nCamera, nLight, nStrategies;
    if (depth == 0) {
        // Ray from the camera directly hits a light.
        nCamera = 2;
        nLight = 0;
        nStrategies = 1;
    } else {
        // Otherwise
        nStrategies = depth + 2;
        nLight = std::min((int)(sampler.get1D() * nStrategies), nStrategies - 1);
        nCamera = nStrategies - nLight;
    }

    // Sample pixel
    const int width  = camera->film()->resolution().x();
    const int height = camera->film()->resolution().y();
    const double px = std::min(sampler.get1D() * width, (double)(width - EPS));
    const double py = std::min(sampler.get1D() * height, (double)(height - EPS));
    *pixel = Point2d(px, py);

    const int x = std::min((int)pixel->x(), width - 1);
    const int y = std::min((int)pixel->y(), height - 1);
    const Point2d randFilm(pixel->x() - x, pixel->y() - y);

    // Sample camera subpath
    Point2i pixel2i(x, y);
    Vertex *cameraPath = arena.allocate<Vertex[]>(nCamera);
    if (nCamera != calcCameraSubpath(scene, sampler, arena, nCamera, *camera, pixel2i, randFilm, cameraPath)) {
        return Spectrum(0.0);
    }

    // Sample light subpath
    sampler.startStream(LIGHT_STREAM);
    Vertex *lightPath = arena.allocate<Vertex[]>(nLight);
    if (nLight != calcLightSubpath(scene, sampler, arena, nLight, lightDist, lightPath)) {
        return Spectrum(0.0);
    }

    // Connect
    sampler.startStream(CONNECT_STREAM);
    return connectBDPT(scene, lightPath, cameraPath, nLight, nCamera,
                       lightDist, *camera, sampler, pixel) * nStrategies;
}

}  // namespace spica
