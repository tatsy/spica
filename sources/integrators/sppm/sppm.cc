#define SPICA_API_EXPORT
#include "sppm.h"

#include <cstdio>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <atomic>

#include "core/memory.h"
#include "core/parallel.h"
#include "core/renderparams.h"
#include "core/tmo.h"
#include "core/interaction.h"
#include "core/sampling.h"
#include "core/film.h"
#include "core/scene.h"
#include "core/camera.h"
#include "core/sampler.h"
#include "core/bsdf.h"
#include "core/bssrdf.h"
#include "core/phase.h"
#include "core/mis.h"

namespace spica {

struct SPPMIntegrator::SPPMPixel {
    Vector3d wo;
    Spectrum Ld = Spectrum(0.0, 0.0, 0.0);
    struct VisiblePoint {
        VisiblePoint() {}
        VisiblePoint(const Point3d& p_, const Vector3d& wo_, const BSDF* bsdf_,
                     const Spectrum& beta_)
            : p{ p_ }
            , wo{ wo_ }
            , bsdf{ bsdf_ }
            , beta{ beta_ } {
        }

        Point3d p;
        Vector3d wo;
        const BSDF* bsdf = nullptr;
        Spectrum beta;
    } vp;

    Spectrum tau;
    AtomicDouble phi[Spectrum::channels];
    double r2 = 0.0;
    double n  = 0;
    std::atomic<int> m = { 0 };
};

const double SPPMIntegrator::kAlpha_ = 0.7;

SPPMIntegrator::SPPMIntegrator(const std::shared_ptr<Sampler>& sampler)
    : Integrator{}
    , sampler_{ sampler } {
}

SPPMIntegrator::SPPMIntegrator(RenderParams &params)
    : SPPMIntegrator{std::static_pointer_cast<Sampler>(params.getObject("sampler", true))} {
}

SPPMIntegrator::~SPPMIntegrator() {
}

void SPPMIntegrator::render(const std::shared_ptr<const Camera>& camera,
                            const Scene& scene,
                            RenderParams& params) {
    Integrator::render(camera, scene, params);
    const int width  = camera_->film()->resolution().x();
    const int height = camera_->film()->resolution().y();
    const int numPoints = width * height;

    // Initialize hitpoints
    std::vector<SPPMPixel> pixels(numPoints);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int id = y * width + x;
            pixels[id].n = 0;
        }
    }

    // Compute light power distribution
    Distribution1D lightDistrib = calcLightPowerDistrib(scene);

    // Initialize random number samplers
    const int nThreads = numSystemThreads();
    auto samplers = std::vector<std::unique_ptr<Sampler>>(nThreads);
    auto arenas   = std::vector<MemoryArena>(nThreads);
    for (int i = 0; i < nThreads; i++) {
        unsigned int seed = (unsigned int)time(0) + i;
        samplers[i] = sampler_->clone(seed);
    }

    const int numSamples = params.getInt("sampleCount");
    const int castPhotons = params.getInt("globalPhotons");
    for (int t = 0; t < numSamples; t++) {
        std::cout << "--- Iteration No." << (t + 1) << " ---" << std::endl;

        // 1st pass: Trace rays from camera
        traceRays(scene, params, samplers, arenas, pixels);

        // 2nd pass: Trace photons from light source
        tracePhotons(scene, params, samplers, arenas,
                     lightDistrib, castPhotons);

        // Update pixel values
        for (int i = 0; i < numPoints; i++) {
            SPPMPixel& pixel = pixels[i];
            if (pixel.m > 0) {
                double nnew  = static_cast<int>(pixel.n + kAlpha_ * pixel.m);
                double r2new = pixel.r2 * nnew / (pixel.n + pixel.m);
                Spectrum phi;
                for (int ch = 0; ch < Spectrum::channels; ch++) {
                    phi.ref(ch) = (double)pixel.phi[ch];
                }
                pixel.tau = (pixel.tau + pixel.vp.beta * phi) * 
                            (r2new / pixel.r2);

                pixel.n  = nnew;
                pixel.r2 = r2new;
                pixel.m = 0;
                for (int ch = 0; ch < Spectrum::channels; ch++) {
                    pixel.phi[ch] = 0.0;
                }
            }
            pixel.vp.beta = Spectrum(0.0);
            pixel.vp.bsdf = nullptr;
        }

        // Reset memory arenas
        for (int k = 0; k < nThreads; k++) {
            arenas[k].reset();
        }

        // Save temporal image
        const int totalPhotons = (t + 1) * castPhotons;
        Image image(width, height);
        image.fill(RGBSpectrum(0.0, 0.0, 0.0));
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                SPPMPixel& pixel = pixels[y * width + x];
                Spectrum L = pixel.Ld / (t + 1);
                L += pixel.tau / (totalPhotons * PI * pixel.r2 + EPS);
                image.pixel(width - x - 1, y) = L;
            }
        }
        camera_->film()->setImage(image);
        camera_->film()->save(t + 1);
    }
}

void SPPMIntegrator::constructHashGrid(std::vector<SPPMPixel>& pixels,
                                       int imageW, int imageH) const {
    // Clear current data
    hashgrid_.clear();

    const int numPoints = static_cast<int>(pixels.size());

    // Compute bounding box
    Bounds3d bbox;
    for (int i = 0; i < numPoints; i++) {
        bbox.merge(pixels[i].vp.p);
    }

    // Heuristic for initial radius
    Vector3d boxSize = bbox.posMax() - bbox.posMin();
    const double irad = ((boxSize.x() + boxSize.y() + boxSize.z()) / 3.0) /
                        ((imageW + imageH) / 2.0) * 2.0;

    // Update initial radius
    Vector3d iradv(irad, irad, irad);
    for (int i = 0; i < numPoints; i++) {
        if (pixels[i].n == 0) {
            pixels[i].r2 = irad * irad;
            pixels[i].n  = 0;
            pixels[i].tau = Spectrum(0.0, 0.0, 0.0);
        }

        bbox.merge(pixels[i].vp.p + iradv);
        bbox.merge(pixels[i].vp.p - iradv);
    }
        
    // Make each grid cell two times larger than the initial radius
    const double hashScale = 1.0 / (irad * 2.0);
    const int hashSize = numPoints;

    hashgrid_.init(hashSize, hashScale, bbox);

    // Set hit points to the grid
    for (int i = 0; i < numPoints; i++) {
        Point3d boxMin = pixels[i].vp.p - iradv;
        Point3d boxMax = pixels[i].vp.p + iradv;
        hashgrid_.add(&pixels[i], boxMin, boxMax);
    }
}

void SPPMIntegrator::traceRays(const Scene& scene,
                               RenderParams& params,
                               const std::vector<std::unique_ptr<Sampler>>& samplers,
                               std::vector<MemoryArena>& arenas,
                               std::vector<SPPMPixel>& hpoints) const {
    const int width  = camera_->film()->resolution().x();
    const int height = camera_->film()->resolution().y();
    const int numPixels = static_cast<int>(hpoints.size());
    const int nThreads = numSystemThreads();

    // Generate a ray to cast
    std::cout << "Tracing rays from camera ..." << std::endl;

    std::atomic<int> proc(0);
    const int tasksThread = (numPixels + nThreads - 1) / nThreads;
    parallel_for(0, numPixels, [&](int pid) {
        const int threadID = getThreadID();

        const auto &sampler = samplers[threadID];
        sampler->startPixel();

        const int px  = pid % width;
        const int py  = pid / width;
        const Point2d randFilm = sampler->get2D();
        const Point2d randLens = sampler->get2D();
        const Ray ray = camera_->spawnRay(Point2i(px, py), randFilm, randLens);


        pathTrace(scene, params, ray, *samplers[threadID],
                    arenas[threadID], &hpoints[pid]);

        proc++;
        if (proc % width == 0) {
            printf("%6.2f %% processed...\r", 100.0 * proc / numPixels);
            fflush(stdout);
        }
    });
    printf("\nFinish !!\n");

    // Construct hash grid
    constructHashGrid(hpoints, width, height);
    std::cout << "Hash grid constructed !!" << std::endl;
}

void SPPMIntegrator::tracePhotons(const Scene& scene, 
                                  RenderParams& params,
                                  const std::vector<std::unique_ptr<Sampler>>& samplers,
                                  std::vector<MemoryArena>& arenas,
                                  const Distribution1D& lightDistrib,
                                  const int numPhotons) const {
    std::cout << "Shooting photons ..." << std::endl;

    // Distribute tasks
    const int nThreads = numSystemThreads();
    const int tasksThread = (numPhotons + nThreads - 1) / nThreads;

    // Trace photons
    std::atomic<int> proc(0);
    parallel_for(0, numPhotons, [&](int p) {
        const int threadID = getThreadID();
        const std::unique_ptr<Sampler>& sampler = samplers[threadID];
        sampler->startPixel();

        // Sample light source
        double lightPdf;
        int lightID = lightDistrib.sampleDiscrete(sampler->get1D(), &lightPdf);
        const std::shared_ptr<Light>& light = scene.lights()[lightID];

        Point2d rand0 = sampler->get2D();
        Point2d rand1 = sampler->get2D();
        Ray photonRay;
        Normal3d nLight;
        double pdfPos, pdfDir;
        Spectrum Le = light->sampleLe(rand0, rand1, &photonRay,
                                        &nLight, &pdfPos, &pdfDir);

        if (pdfPos != 0.0 && pdfDir != 0.0 && !Le.isBlack()) {
            Spectrum beta = (vect::absDot(nLight, photonRay.dir()) * Le) /
                            (lightPdf * pdfPos * pdfDir);
            if (!beta.isBlack()) {
                tracePhotonsSub(scene, params, photonRay, beta,
                                *sampler, arenas[threadID]);
            }
        }

        proc++;
        if (proc % 100 == 0) {
            printf("%6.2f %% processed ...\r", 100.0 * proc / numPhotons);
            fflush(stdout);
        }
    });
    printf("%6.2f %% processed ...\r", 100.0);
    printf("\nFinish !!\n");
    fflush(stdout);
}

void SPPMIntegrator::tracePhotonsSub(const Scene& scene,
                                     RenderParams& params,
                                     const Ray& r,
                                     const Spectrum& b,
                                     Sampler& sampler,
                                     MemoryArena& arena) const {
    Ray ray(r);
    Spectrum beta(b);
    SurfaceInteraction isect;
    const int maxBounces = params.getInt("maxDepth");
    for (int bounces = 0; bounces < maxBounces; bounces++) {
        bool isIntersect = scene.intersect(ray, &isect);

        // Sample participating media
        MediumInteraction mi;
        if (ray.medium()) beta *= ray.medium()->sample(ray, sampler, arena, &mi);
        if (beta.isBlack()) break;

        if (mi.isValid()) {
            if (bounces >= maxBounces) break;

            Vector3d wo = -ray.dir();
            Vector3d wi;
            mi.phase()->sample(wo, &wi, sampler.get2D());
            ray = mi.spawnRay(wi);        
        } else {
            if (!isIntersect) break;

            if (bounces > 0) {
                std::vector<SPPMPixel*> results;

                results = hashgrid_[isect.pos()];
            
                for (SPPMPixel* pixel : results) {
                    if ((pixel->vp.p - isect.pos()).squaredNorm() > pixel->r2) {
                        continue;
                    }

                    if (!pixel->vp.bsdf) {
                        continue;
                    }

                    Vector3d wi = -ray.dir();
                    Spectrum phi = beta * pixel->vp.bsdf->f(pixel->vp.wo, wi);
                    for (int ch = 0; ch < Spectrum::channels; ch++) {
                        pixel->phi[ch].add(phi[ch]);
                    }
                    ++pixel->m;
                }
            }
    
            isect.setScatterFuncs(ray, arena);
            if (!isect.bsdf()) {
                bounces--;
                ray = isect.spawnRay(ray.dir());
                continue;
            }
            const BSDF& bsdf = *isect.bsdf();

            Vector3d wi, wo = -ray.dir();
            double pdf;
            BxDFType sampledType;
            Spectrum ref = bsdf.sample(wo, &wi, sampler.get2D(), &pdf,
                                       BxDFType::All, &sampledType);

            if (pdf == 0.0 || ref.isBlack()) break;

            Spectrum bnew = beta * ref * vect::absDot(wi, isect.normal()) / pdf;

            double continueProb = std::min(1.0, bnew.luminance() / beta.luminance());
            if (sampler.get1D() > continueProb) break;
            beta = bnew / continueProb;
            ray = isect.spawnRay(wi);

            // Account for BSSRDF
            if (isect.bssrdf() && (sampledType & BxDFType::Transmission) != BxDFType::None) {
                SurfaceInteraction pi;
                Spectrum S = isect.bssrdf()->sample(scene, sampler.get1D(),
                    sampler.get2D(), arena, &pi, &pdf);
            
                if (S.isBlack() || pdf == 0.0) break;
                beta *= S / pdf;

                Spectrum f = pi.bsdf()->sample(pi.wo(), &wi, sampler.get2D(), &pdf,
                                               BxDFType::All, &sampledType);
                if (f.isBlack() || pdf == 0.0) break;
                beta *= f * vect::absDot(wi, pi.normal()) / pdf;

                ray = pi.spawnRay(wi);
            }
        }
    }
}

void SPPMIntegrator::pathTrace(const Scene& scene,
                               RenderParams& params,
                               const Ray& r,
                               Sampler& sampler,
                               MemoryArena& arena,
                               SPPMPixel* pixel) const {
    Ray ray(r);
    Spectrum beta(1.0);
    bool specularBounce = false;
    const int maxBounces = params.getInt("maxDepth");
    for (int bounces = 0; bounces < maxBounces; bounces++) {
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        // Sample participating media
        MediumInteraction mi;
        if (ray.medium()) beta *= ray.medium()->sample(ray, sampler, arena, &mi);
        if (beta.isBlack()) break;

        if (mi.isValid()) {
            pixel->Ld += beta * uniformSampleOneLight(mi, scene, arena, sampler, true);

            if (bounces >= maxBounces) break;

            Vector3d wo = -ray.dir();
            Vector3d wi;
            mi.phase()->sample(wo, &wi, sampler.get2D());
            ray = mi.spawnRay(wi);
        } else {
            // If not intesect the scene, sample direct light.
            if (!isIntersect) {
                for (const auto& light : scene.lights()) {
                    pixel->Ld += beta * light->Le(ray);
                }
                break;
            }

            isect.setScatterFuncs(ray, arena);
            if (!isect.bsdf()) {
                ray = isect.spawnRay(ray.dir());
                bounces--;
                continue;
            }
            const BSDF& bsdf = *isect.bsdf();

            const Vector3d wo = -ray.dir();
            if (bounces == 0 || specularBounce) {
                pixel->Ld += beta * isect.Le(wo);
            }
            pixel->Ld += 
                beta * uniformSampleOneLight(isect, scene, arena, sampler);

            bool isDiffuse = bsdf.numComponents(
                BxDFType::Diffuse | BxDFType::Reflection | BxDFType::Transmission) > 0;       
            bool isGlossy  = bsdf.numComponents(
                BxDFType::Glossy | BxDFType::Reflection | BxDFType::Transmission) > 0;

            if (isDiffuse || (isGlossy && bounces == maxBounces - 1)) {
                pixel->vp = { isect.pos(), wo, &bsdf, beta };
                //break;
            }

            if (bounces < maxBounces - 1) {
                Vector3d wi;
                double pdf;
                BxDFType sampledType;
                Spectrum ref = bsdf.sample(wo, &wi, sampler.get2D(), &pdf,
                                           BxDFType::All, &sampledType);

                if (pdf == 0.0 || ref.isBlack()) break;

                beta *= ref * vect::absDot(wi, isect.normal()) / pdf;
                specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
                if (beta.luminance() < 0.25) {
                    double continueProb = std::min(1.0 , beta.luminance());
                    if (sampler.get1D() > continueProb) break;
                    beta /= continueProb;
                }           
                ray = isect.spawnRay(wi);

                // Account for BSSRDF
                if (isect.bssrdf() && (sampledType & BxDFType::Transmission) != BxDFType::None) {
                    SurfaceInteraction pi;
                    Spectrum S = isect.bssrdf()->sample(scene, sampler.get1D(),
                        sampler.get2D(), arena, &pi, &pdf);

                    if (S.isBlack() || pdf == 0.0) break;
                    beta *= S / pdf;

                    pixel->Ld += beta * uniformSampleOneLight(pi, scene, arena, sampler);

                    Spectrum f = pi.bsdf()->sample(pi.wo(), &wi, sampler.get2D(),
                        &pdf, BxDFType::All, &sampledType);
                    if (f.isBlack() || pdf == 0.0) break;
                    beta *= f * vect::absDot(wi, pi.normal()) / pdf;

                    specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
                    ray = pi.spawnRay(wi);
                }
            }
        }
    }
}

}  // namespace spica
