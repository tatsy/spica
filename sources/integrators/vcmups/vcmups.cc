#define SPICA_API_EXPORT
#include "vcmups.h"

#include <fstream>
#include <mutex>

#include "core/memory.h"
#include "core/parallel.h"
#include "core/sampler.h"
#include "core/camera.h"
#include "core/film.h"
#include "core/light.h"
#include "core/scene.h"
#include "core/sampling.h"
#include "core/bsdf.h"
#include "core/mis.h"
#include "core/visibility_tester.h"

#include "integrators/bidirectional.h"
#include "integrators/photonmap.h"

std::mutex mtx;

namespace spica {

namespace {

double calcWeightSum(const Scene& scene,
                     Vertex* lightPath, Vertex* cameraPath,
                     Vertex& sampled,
                     int lightID, int cameraID,
                     const Distribution1D& lightDist,
                     double *riLight, double *riCamera) {
    // Single bounce connection.
    if (lightID + cameraID == 2) return 1.0;

    // To handle specular reflection, zero contribution is remapped to 1.0.
    auto remap0 = [](double f) -> double { return f != 0.0 ? f : 1.0; };

    // Take current and previous sample.
    Vertex* vl = lightID > 0 ? &lightPath[lightID - 1] : nullptr;
    Vertex* vc = cameraID > 0 ? &cameraPath[cameraID - 1] : nullptr;
    Vertex* vlMinus = lightID > 1 ? &lightPath[lightID - 2] : nullptr;
    Vertex* vcMinus = cameraID > 1 ? &cameraPath[cameraID - 2] : nullptr;

    // Temporal swaps in this function scope.
    ScopedAssignment<Vertex> a1;
    if (lightID == 1) {
        a1 = { vl, sampled };
    } else if (cameraID == 1) {
        a1 = { vc, sampled };
    }

    ScopedAssignment<bool> a2, a3;
    if (vc) a2 = { &vc->delta, false };
    if (vl) a3 = { &vl->delta, false };

    ScopedAssignment<double> a4;
    if (vc) {
        a4 = { &vc->pdfRev, lightID > 0
            ? vl->pdf(scene, vlMinus, *vc)
            : vc->pdfLightOrigin(scene, *vcMinus, lightDist) };
    }

    ScopedAssignment<double> a5;
    if (vcMinus) {
        a5 = { &vcMinus->pdfRev, lightID > 0 ? vc->pdf(scene, vl, *vcMinus)
            : vc->pdfLight(scene, *vcMinus) };
    }

    ScopedAssignment<double> a6;
    if (vl) {
        a6 = { &vl->pdfRev, vc->pdf(scene, vcMinus, *vl) };
    }
    ScopedAssignment<double> a7;
    if (vlMinus) {
        a7 = { &vlMinus->pdfRev, vl->pdf(scene, vc, *vlMinus) };
    }

    // Compute sum of path contributions of the same length.
    double sumRi = 0.0;
    double ri = 1.0;
    for (int i = cameraID - 1; i > 0; i--) {
        ri *= remap0(cameraPath[i].pdfRev) / remap0(cameraPath[i].pdfFwd);
        if (cameraPath[i].delta || cameraPath[i - 1].delta) continue;

        if (i == 1) {
            if (riCamera) *riCamera = ri;
        } else {
            sumRi += ri;
        }
    }

    ri = 1.0;
    for (int i = lightID - 1; i >= 0; i--) {
        ri *= remap0(lightPath[i].pdfRev) / remap0(lightPath[i].pdfFwd);
        bool isDeltaLight = i > 0 ? lightPath[i - 1].delta
            : lightPath[0].isDeltaLight();
        if (lightPath[i].delta || isDeltaLight) continue;

        if (i == 0) {
            if (riLight) *riLight = ri;
        } else {
            sumRi += ri;
        }
    }

    return 1.0 + sumRi;
}

Spectrum connectVCM(const Scene& scene,
                    Vertex* lightPath, Vertex* cameraPath,
                    int lightID, int cameraID,
                    int nLight, int nCamera,
                    const std::vector<std::unique_ptr<PhotonMap>> &photonMaps,
                    int lookupSize, const double lookupRadius, int numPixels,
                    const Distribution1D& lightDist,
                    const Camera& camera, Sampler& sampler, Point2d* pRaster,
                    double* misWeight) {

    Spectrum L_MC(0.0);  // Radiance for BDPT
    Spectrum L_DE(0.0);  // Radiance for Photon map

    if (cameraID > 1 && lightID != 0 &&
        cameraPath[cameraID - 1].type == VertexType::Light) {
        return Spectrum(0.0);
    }

    Vertex sampled;
    if (lightID == 0) {
        // No light vertex (camera path hits a light)
        const Vertex& vc = cameraPath[cameraID - 1];
        if (vc.isLight()) L_MC = vc.Le(scene, cameraPath[cameraID - 2]) * vc.beta;
    } else if (cameraID == 1) {
        // Camera vertex is on the sensor
        const Vertex& vl = lightPath[lightID - 1];
        if (vl.isConnectible()) {
            VisibilityTester vis;
            Vector3d wi;
            double pdf;
            Spectrum Wi = camera.sampleWi(vl.getInteraction(), sampler.get2D(),
                                          &wi, &pdf, pRaster, &vis);
            if (pdf > 0.0 && !Wi.isBlack()) {
                sampled = Vertex::createCamera(&camera, vis.p2(), Wi / pdf);
                L_MC = vl.beta * vl.f(sampled) * sampled.beta;
                if (vl.isOnSurface()) L_MC *= vect::absDot(wi, vl.normal());
                if (!L_MC.isBlack()) L_MC *= vis.transmittance(scene, sampler);
            }
        }
    } else if (lightID == 1) {
        // Direct illumination
        const Vertex& vc = cameraPath[cameraID - 1];

        // Monte Carlo
        if (vc.isConnectible()) {
            double lightPdf;
            VisibilityTester vis;
            Vector3d wi;
            double pdf;
            int id = lightDist.sampleDiscrete(sampler.get1D(), &lightPdf);
            const auto& l = scene.lights()[id];

            Spectrum lightWeight = l->sampleLi(vc.getInteraction(), sampler.get2D(),
                                               &wi, &pdf, &vis);

            if (pdf > 0.0 && !lightWeight.isBlack()) {
                EndpointInteraction ei(vis.p2(), l.get());
                sampled = Vertex::createLight(ei, lightWeight / (pdf * lightPdf), 0.0);
                sampled.pdfFwd = sampled.pdfLightOrigin(scene, vc, lightDist);
                L_MC = vc.beta * vc.f(sampled) * sampled.beta;

                if (vc.isOnSurface()) L_MC *= vect::absDot(wi, vc.normal());
                if (!L_MC.isBlack()) L_MC *= vis.transmittance(scene, sampler);
            }
        }

        // Photon density estimate
        for (int c = cameraID - 1; c >= 1; c--) {
            const Vertex &vcMid = cameraPath[c];
            const int l = lightID + (cameraID - c - 1);
            if (vcMid.isConnectible() && l < nLight) {
                const SurfaceInteraction *intr = vcMid.si();
                if (intr) {
                    const bool isDiffuse = intr->bsdf()->hasType(BxDFType::Diffuse | BxDFType::Reflection);
                    if (isDiffuse) {
                        L_DE += vcMid.beta * photonMaps[l]->evaluateL(*intr, lookupSize, lookupRadius);
                    }
                }
            }
        }

    } else {
        // Ordinary connection case
        const Vertex& vc = cameraPath[cameraID - 1];
        const Vertex& vl = lightPath[lightID - 1];

        // Monte Carlo
        if (vc.isConnectible() && vl.isConnectible()) {
            L_MC = vc.beta * vc.f(vl) * vl.f(vc) * vl.beta;
            if (!L_MC.isBlack()) L_MC *= G(scene, sampler, vl, vc);
        }

        // Photon density estimate
        for (int c = cameraID - 1; c >= 1; c--) {
            const Vertex &vcMid = cameraPath[c];
            const int l = lightID + (cameraID - c - 1);
            if (vcMid.isConnectible() && l < nLight) {
                const SurfaceInteraction *intr = vcMid.si();
                if (intr) {
                    const bool isDiffuse = intr->bsdf()->hasType(BxDFType::Diffuse | BxDFType::Reflection);
                    if (isDiffuse) {
                        L_DE += vcMid.beta * photonMaps[l]->evaluateL(*intr, lookupSize, lookupRadius);
                    }
                }
            }
        }

    }

    double sumW_DE = 0.0;
    double riCamera = 0.0;
    double riLight = 0.0;
    if (!L_MC.isBlack()) {
        sumW_DE = calcWeightSum(scene, lightPath, cameraPath, sampled,
                                lightID, cameraID, lightDist, &riLight, &riCamera);
    }
    const double sumW_MC = sumW_DE + riCamera + riLight;

    if (sumW_MC == 0.0) {
        if (lightID + cameraID > 1) {
            return L_DE / numPixels / (lightID + cameraID - 1);
        }
        return Spectrum(0.0);
    }

    // Kernel sampling
    const double k = 1.1;
    const Point2d randDisk = sampleConcentricDisk(sampler.get2D());
    const double accumW = (1.0 - 2.0 / (3.0 * k)) * (PI * lookupRadius * lookupRadius);
    const double kernelW = std::max(0.0, 1.0 - std::hypot(randDisk.x(), randDisk.y()) / k) / accumW;

    double misW_MC = 0.0, misW_DE = 0.0;
    if ((!L_MC.isBlack() && !L_DE.isBlack()) || cameraID == 1) {
        misW_MC = kernelW / (sumW_MC * kernelW + sumW_DE * numPixels);
        misW_DE = numPixels / (sumW_MC * kernelW + sumW_DE * numPixels);
    } else if (!L_MC.isBlack()) {
        misW_MC = 1.0 / sumW_MC;
    } else if (!L_DE.isBlack()) {
        misW_DE = 1.0 / sumW_DE;
    }

    L_MC *= misW_MC;
    L_DE *= misW_DE / numPixels;

    if (misWeight) *misWeight = misW_MC;

    return L_MC + L_DE;
}

}  // Anonymous namespace

VCMUPSIntegrator::VCMUPSIntegrator(const std::shared_ptr<Sampler> &sampler, double alpha)
    : Integrator{ }
    , sampler_{ sampler }
    , alpha_{ alpha } {
}

VCMUPSIntegrator::VCMUPSIntegrator(RenderParams &params)
    : VCMUPSIntegrator{ std::static_pointer_cast<Sampler>(params.getObject("sampler")),
                        params.getDouble("alpha", 0.8) } {
}

void VCMUPSIntegrator::initialize(const std::shared_ptr<const Camera> &camera,
    const Scene& scene,
    RenderParams& params,
    Sampler& sampler) {
    // Compute global radius
    const Bounds3d bounds = scene.worldBound();
    lookupRadiusScale_ = (bounds.posMax() - bounds.posMin()).norm() * 0.5;
}

void VCMUPSIntegrator::loopStarted(const std::shared_ptr<const Camera> &camera,
    const Scene& scene,
    RenderParams& params,
    Sampler& sampler) {
}

void VCMUPSIntegrator::loopFinished(const std::shared_ptr<const Camera> &camera,
    const Scene& scene,
    RenderParams& params,
    Sampler& sampler) {
    // Scale global radius
    lookupRadiusScale_ = std::max(1.0, lookupRadiusScale_ * alpha_);
}

void VCMUPSIntegrator::render(const std::shared_ptr<const Camera> &camera,
                              const Scene &scene,
                              RenderParams &params) {
    // Initialization
    const int width = camera->film()->resolution().x();
    const int height = camera->film()->resolution().y();
    initialize(camera, scene, params, *sampler_);

    const int numThreads = numSystemThreads();
    auto samplers = std::vector<std::unique_ptr<Sampler>>(numThreads);
    auto globalArenas = std::vector<MemoryArena>(numThreads);
    auto subArenas = std::vector<MemoryArena>(numThreads);

    Distribution1D lightDist = calcLightPowerDistrib(scene);

    const int numPixels = width * height;
    const int numSamples = params.getInt("sampleCount");
    const int maxBounces = params.getInt("maxDepth");

    std::vector<int> lightPathLengths(numPixels);
    std::vector<Vertex*> lightPaths(numPixels);

    for (int i = 0; i < numSamples; i++) {
        // Event before loop
        loopStarted(camera, scene, params, *sampler_);

        // Prepare samplers
        if (i % numThreads == 0) {
            for (int t = 0; t < numThreads; t++) {
                auto seed = static_cast<unsigned int>(time(0) + t);
                samplers[t] = sampler_->clone(seed);
            }
        }

        // Sample camera and light paths
        MsgInfo("Sampling paths");
        std::mutex mtx;
        std::atomic<int> proc(0);
        parallel_for(0, numPixels, [&](int pid) {
            const int threadID = getThreadID();
            const auto &sampler = samplers[threadID];
            sampler->startPixel();

            lightPaths[pid] = globalArenas[threadID].allocate<Vertex[]>(maxBounces + 1);
            lightPathLengths[pid] = calcLightSubpath(scene, *sampler, globalArenas[threadID],
                                                     maxBounces + 1, lightDist, lightPaths[pid]);

            proc++;
            if (proc % 1000 == 0 || proc == numPixels) {
                printf("\r[ %d / %d ] %6.2f %% processed...", i + 1, numSamples, 100.0 * proc / numPixels);
                fflush(stdout);
            }
        });
        printf("\n");

        // Photon maps for each bounce count
        MsgInfo("Constructing photon maps...");
        std::vector<std::unique_ptr<PhotonMap>> photonMaps(maxBounces + 1);
        for (int b = 1; b < maxBounces + 1; b++) {
            photonMaps[b] = std::make_unique<PhotonMap>(PhotonMapType::Global);
            std::vector<Photon> photons;
            for (int p = 0; p < numPixels; p++) {
                if (b < lightPathLengths[p]) {
                    Vertex &v = lightPaths[p][b];
                    const SurfaceInteraction *intr = v.si();
                    if (intr) {
                        const bool isDiffuse = intr->bsdf()->hasType(BxDFType::Diffuse | BxDFType::Reflection);
                        if (isDiffuse) {
                            photons.emplace_back(v.pos(), v.beta, intr->wo(), intr->normal());
                        }
                    }
                }
            }
            MsgInfo("#bounce: %d, #photons: %d", b, (int)photons.size());
            photonMaps[b]->construct(photons);
        }

        // Density estimation
        proc.store(0);
        parallel_for(0, numPixels, [&](int pid) {
            const int threadID = getThreadID();
            const auto &sampler = samplers[threadID];
            sampler->startPixel();

            const int y = pid / width;
            const int x = pid % width;

            const int nLight = lightPathLengths[pid];
            const Point2d randFilm = sampler->get2D();

            Vertex *cameraPath = subArenas[threadID].allocate<Vertex[]>(maxBounces + 2);
            const int nCamera = calcCameraSubpath(scene, *sampler, subArenas[threadID], maxBounces + 2,
                                                  *camera, Point2i(x, y), randFilm, cameraPath);

            Spectrum L(0.0);
            for (int cid = 1; cid <= nCamera; cid++) {
                for (int lid = 0; lid <= nLight; lid++) {
                    int depth = cid + lid - 2;
                    if ((cid == 1 && lid == 1) || (depth < 0) || (depth > maxBounces)) continue;

                    Point2d pFilm = Point2d(x + randFilm.x(), y + randFilm.y());
                    double misWeight = 0.0;

                    const int lookupSize = params.getInt("lookupSize", 32);
                    const double lookupRadius = params.getDouble("lookupRadius", 0.125) * lookupRadiusScale_;
                    Spectrum Lpath = connectVCM(scene, lightPaths[pid], cameraPath,
                                                lid, cid, nLight, nCamera, photonMaps,
                                                lookupSize, lookupRadius, numPixels,
                                                lightDist, *camera, *sampler, &pFilm, &misWeight);
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
            if (proc % 1000 == 0 || proc == numPixels) {
                printf("\r[ %d / %d ] %6.2f %% processed...", i + 1, numSamples, 100.0 * proc / numPixels);
                fflush(stdout);
            }

            // Reset sub arena.
            subArenas[threadID].reset();
        });
        printf("\n");

        camera->film()->save(i + 1, 1.0 / (i + 1));

        // Reset global arena
        for (auto &arena: globalArenas) {
            arena.reset();
        }

        // Event after loop
        loopFinished(camera, scene, params, *sampler_);
    }
    std::cout << "Finish!!" << std::endl;
}

}  // namespace spica