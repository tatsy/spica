#define SPICA_API_EXPORT
#include "bdpt.h"

#include <mutex>

#include "core/ray.h"
#include "core/interaction.h"
#include "core/sampling.h"
#include "core/memory.h"
#include "core/parallel.h"
#include "core/renderparams.h"
#include "core/film.h"
#include "core/scene.h"
#include "core/light.h"
#include "core/camera.h"
#include "core/bxdf.h"
#include "core/phase.h"
#include "core/bsdf.h"
#include "core/medium.h"
#include "core/sampler.h"
#include "core/mis.h"
#include "core/visibility_tester.h"

#include "integrators/bidirectional.h"

namespace spica {

BDPTIntegrator::BDPTIntegrator(const std::shared_ptr<Sampler>& sampler)
    : Integrator{ }
    , sampler_{ sampler } {
}

BDPTIntegrator::BDPTIntegrator(spica::RenderParams &params)
    : BDPTIntegrator{ std::static_pointer_cast<Sampler>(params.getObject("sampler")) } {
}

BDPTIntegrator::~BDPTIntegrator() {
}

void BDPTIntegrator::render(const std::shared_ptr<const Camera>& camera,
                            const Scene& scene,
                            RenderParams& params) {
    // Initialization
    const int width = camera->film()->resolution().x();
    const int height = camera->film()->resolution().y();

    const int numThreads = numSystemThreads();
    auto samplers = std::vector<std::unique_ptr<Sampler>>(numThreads);
    auto arenas   = std::vector<MemoryArena>(numThreads);

    Distribution1D lightDist = calcLightPowerDistrib(scene);

    const int numPixels = width * height;
    const int numSamples = params.getInt("sampleCount");
    const int maxBounces = params.getInt("maxDepth");
    for (int i = 0; i < numSamples; i++) {
        // Prepare samplers
        if (i % numThreads == 0) {
            for (int t = 0; t < numThreads; t++) {
                auto seed = static_cast<unsigned int>(time(0) + t);
                samplers[t] = sampler_->clone(seed);
            }
        }

        std::mutex mtx;
        std::atomic<int> proc(0);
        parallel_for(0, numPixels, [&](int pid) {
            const int threadID = getThreadID();
            const auto &sampler = samplers[threadID];
            sampler->startPixel();

            const int y = pid / width;
            const int x = pid % width;
            const Point2d randFilm = sampler->get2D();

            auto cameraPath = std::make_unique<Vertex[]>(maxBounces + 2);
            auto lightPath  = std::make_unique<Vertex[]>(maxBounces + 1);

            const int nCamera = calcCameraSubpath(scene, *sampler, arenas[threadID],
                              maxBounces + 2, *camera, Point2i(x, y), randFilm,
                              cameraPath.get());
            const int nLight = calcLightSubpath(scene, *sampler, arenas[threadID],
                             maxBounces + 1, lightDist, lightPath.get());

            Spectrum L(0.0);
            for (int cid = 1; cid <= nCamera; cid++) {
                for (int lid = 0; lid <= nLight; lid++) {
                    int depth = cid + lid - 2;
                    if ((cid == 1 && lid == 1) || (depth < 0) || (depth > maxBounces)) continue;

                    Point2d pFilm = Point2d(x + randFilm.x(), y + randFilm.y());
                    double misWeight = 0.0;

                    Spectrum Lpath = connectBDPT(scene, lightPath.get(), cameraPath.get(),
                        lid, cid, lightDist, *camera, *sampler,
                        &pFilm, &misWeight);
                    if (cid == 1 && !Lpath.isBlack()) {
                        pFilm = Point2d(width - pFilm.x(), pFilm.y());
                        mtx.lock();
                        camera->film()->addPixel(pFilm, Lpath);
                        mtx.unlock();
                    } else {
                        L += Lpath;
                    }
                }
            }
            camera->film()->addPixel(Point2i(width - x - 1, y), randFilm, L);

            proc++;
            if (proc % 1000 == 0) {
                printf("\r[ %d / %d ] %6.2f %% processed...", i + 1, numSamples, 100.0 * proc / numPixels);
                fflush(stdout);
            }
        });

        camera->film()->saveMLT(1.0 / (i + 1), i + 1);

        for (int t = 0; t < numThreads; t++) {
            arenas[t].reset();
        }
    }
    std::cout << "Finish!!" << std::endl;    
}

}  // namespace spica
