#define SPICA_SPPM_EXPORT
#include "sppm.h"

#include <cstdio>
#include <ctime>
#include <iostream>
#include <algorithm>

#include "renderer_helper.h"

#include "../scenes/scene.h"
#include "../utils/sampler.h"
#include "../camera/camera.h"

#include "../random/random_sampler.h"
#include "../random/random.h"
#include "../random/halton.h"

#include "subsurface_integrator.h"

namespace spica {

    const double SPPMRenderer::kAlpha = 0.7;

    SPPMRenderer::SPPMRenderer()
        : IRenderer()
    {
    }

    SPPMRenderer::~SPPMRenderer()
    {
    }

    void SPPMRenderer::render(const Scene& scene, const Camera& camera,
                              const RenderParameters& params) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        const int numPoints = width * height;

        // Preparation for taking BSSRDF into account
        _integrator->initialize(scene);

        // Initialize hitpoints
        std::vector<SPPMPixel> hpoints(numPoints);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int id = y * width + x;
                hpoints[id].x = x;
                hpoints[id].y = y;
                hpoints[id].n = 0;
            }
        }

        // Initialize random number samplers
        RandomSampler* samplers = new RandomSampler[kNumThreads];
        for (int i = 0; i < kNumThreads; i++) {
            switch (params.randomType()) {
            case PSEUDO_RANDOM_TWISTER:
                samplers[i] = Random::factory((unsigned int)i);
                break;

            case QUASI_MONTE_CARLO:
                samplers[i] = Halton::factory(250, true, (unsigned int)i);
                break;

            default:
                std::cerr << "[ERROR] unknown random sampler type !!"
                          << std::endl;
                std::abort();
                break;
            }
        }

        for (int t = 0; t < params.samplePerPixel(); t++) {
            std::cout << "--- Iteration No." << (t + 1) << " ---" << std::endl;
            // 1st pass: Trace rays from camera
            traceRays(scene, camera, params, samplers, hpoints);

            // 2nd pass: Trace photons from light source
            tracePhotons(scene, params, samplers, params.castPhotons());

            // Save temporal image
            Image image(width, height);
            image.fill(Color::BLACK);
            for (int i = 0; i < numPoints; i++) {
                const int px = width - hpoints[i].x - 1;
                const int py = hpoints[i].y;
                image.pixel(px, py) += hpoints[i].radiance() / (t + 1);
            }

            char filename[512];
            sprintf(filename, params.saveFilenameFormat().c_str(), t + 1);
            image.gammaCorrect(1.0 / 2.2);
            image.save(filename);
        }

        // Deallocate memories
        delete[] samplers;
    }

    void SPPMRenderer::constructHashGrid(std::vector<SPPMPixel>& pixels,
                                         const int imageW, const int imageH) {
        // Clear current data
        hashgrid.clear();

        const int numPoints = static_cast<int>(pixels.size());

        // Compute bounding box
        BBox bbox;
        for (int i = 0; i < numPoints; i++) {
            bbox.merge(pixels[i].position);
        }

        // Heuristic for initial radius
        Vector3D boxSize = bbox.posMax() - bbox.posMin();
        const double irad = ((boxSize.x() + boxSize.y() + boxSize.z()) / 3.0) /
                            ((imageW + imageH) / 2.0) * 2.0;

        // Update initial radius
        Vector3D iradv(irad, irad, irad);
        for (int i = 0; i < numPoints; i++) {
            if (pixels[i].n == 0) {
                pixels[i].r2 = irad * irad;
                pixels[i].n = 0;
                pixels[i].flux = Color(0.0, 0.0, 0.0);
            }

            bbox.merge(pixels[i].position + iradv);
            bbox.merge(pixels[i].position - iradv);
        }
        
        // Make each grid cell two times larger than the initial radius
        const double hashScale = 1.0 / (irad * 2.0);
        const int hashSize = numPoints;

        hashgrid.init(hashSize, hashScale, bbox);

        // Set hit points to the grid
        for (int i = 0; i < numPoints; i++) {
            Vector3D boxMin = pixels[i].position - iradv;
            Vector3D boxMax = pixels[i].position + iradv;
            hashgrid.add(&pixels[i], boxMin, boxMax);
        }
    }

    void SPPMRenderer::traceRays(const Scene& scene,
                                 const Camera& camera,
                                 const RenderParameters& params,
                                 RandomSampler* samplers,
                                 std::vector<SPPMPixel>& hpoints) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        const int numPixels = static_cast<int>(hpoints.size());

        // Generate a ray to cast
        std::cout << "Tracing rays from camera ..." << std::endl;

        // Distribute Hitpoints to each thread
        std::vector<std::vector<int> > pixelIDs(kNumThreads);
        for (int i = 0; i < numPixels; i++) {
            pixelIDs[i % kNumThreads].push_back(i);
        }

        int proc = 0;
        const int tasksThread = (numPixels + kNumThreads - 1) / kNumThreads;
        for (int t = 0; t < tasksThread; t++) {
            ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
                if (t < pixelIDs[threadID].size()) {
                    Stack<double> rstk;
                    samplers[threadID].request(&rstk, 250);

                    const int pid = pixelIDs[threadID][t];
                    pathTrace(scene, camera, params, rstk, &hpoints[pid]);
                }
            }

            proc += kNumThreads;
            if (proc % width == 0) {
                printf("%6.2f %% processed...\r", 100.0 * proc / numPixels);
            }
        }
        printf("\nFinish !!\n");

        // Construct k-d tree
        constructHashGrid(hpoints, width, height);
        std::cout << "Hash grid constructed !!" << std::endl << std::endl;
    }

    void SPPMRenderer::tracePhotons(const Scene& scene, 
                                    const RenderParameters& params,
                                    RandomSampler* samplers,
                                    const int numPhotons) const {
        std::cout << "Shooting photons ..." << std::endl;

        // Distribute tasks
        const int tasksThread = (numPhotons + kNumThreads - 1) / kNumThreads;

        // Trace photons
        int proc = 0;
        for (int t = 0; t < tasksThread; t++) {
            ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
                Stack<double> rstk;
                omplock {
                    samplers[threadID].request(&rstk, 250);
                }

                // Sample point on light
                const LightSample ls = scene.sampleLight(rstk);

                // Compute flux
                Color flux = Color(scene.lightArea() * ls.Le() * PI / numPhotons);

                // Prepare ray
                Vector3D nextDir;
                sampler::onHemisphere(ls.normal(), &nextDir);
                Ray ray(ls.position(), nextDir);
                Vector3D prevNormal = ls.normal();

                tracePhotonsRec(scene, ray, params, flux, 0, rstk);
            }

            proc += kNumThreads;
            if (proc % 100 == 0) {
                printf("%6.2f %% processed ...\r", 100.0 * proc / numPhotons);
            }
        }
        printf("\nFinish !!\n\n");
    }

    void SPPMRenderer::tracePhotonsRec(const Scene& scene, const Ray& ray,
                                       const RenderParameters& params,
                                       const Color& flux,
                                       int bounces,
                                       Stack<double>& rstk) const {
        // Too many bounces terminate the recursion
        if (bounces >= params.bounceLimit()) {
            return;
        }
        
        // Request random number
        const double rands[3] = { rstk.pop(), rstk.pop(), rstk.pop() };

        // Remove photons with zero flux
        if (max3(flux.red(), flux.green(), flux.blue()) <= 0.0) {
            return;
        }

        // Intersection check
        Intersection isect;
        if (!scene.intersect(ray, isect)) {
            return;
        }

        const int       objectID = isect.objectId();
        const BSDF&     bsdf = scene.getBsdf(objectID);
        const Color&    refl = bsdf.reflectance();
        const Hitpoint& hpoint = isect.hitpoint();
        const bool into = Vector3D::dot(hpoint.normal(), ray.direction()) < 0.0;
        const Vector3D orientNormal = into ? hpoint.normal()
                                            : -hpoint.normal();

        double photonPdf = 1.0;
        if (bsdf.type() & BsdfType::Lambertian) {
            // Gather hit points
            std::vector<SPPMPixel*> results;
            omplock{
                results = hashgrid[hpoint.position()];
            }

            // Update hit points
            for (int i = 0; i < results.size(); i++) {
                SPPMPixel* pixel = results[i];
                Vector3D v = pixel->position - hpoint.position();
                if (Vector3D::dot(pixel->normal, hpoint.normal()) > EPS &&
                    (v.squaredNorm() <= pixel->r2)) {
                    const double g = (pixel->n * kAlpha + kAlpha) / 
                                     (pixel->n * kAlpha + 1.0);
                    omplock{
                        pixel->r2 *= g;
                        pixel->n += 1;
                        pixel->flux = g * (pixel->flux + 
                                           pixel->weight * flux * INV_PI);
                    }
                }
            }

            // Determine continue or terminate trace with Russian roulette
            bool hoge = false;
            const double prob = (refl.red() + refl.green() + refl.blue()) / 3.0;
            if (rands[0] < prob) {
                photonPdf = prob;
            } else {
                return;
            }
        } 
        
        double samplePdf = 1.0;
        Vector3D nextdir;

        bsdf.sample(ray.direction(), hpoint.normal(),
                    rands[1], rands[2], &nextdir, &samplePdf);
        const Ray nextRay(hpoint.position(), nextdir);
        const Color nextFlux = Color(flux * refl / (photonPdf * samplePdf));

        tracePhotonsRec(scene, nextRay, params, nextFlux, bounces + 1, rstk);
    }

    void SPPMRenderer::pathTrace(const Scene& scene, const Camera& camera,
                                 const RenderParameters& params,
                                 Stack<double>& rstk, SPPMPixel* pixel) {
        Assertion(pixel->x >= 0 && pixel->y >= 0 && 
                  pixel->x < camera.imageW() && 
                  pixel->y < camera.imageH(),
                  "Pixel index out of range");

        CameraSample camSample = camera.sample(pixel->x, pixel->y, rstk);
        Ray ray = camSample.ray();
        const double coeff = camera.sensitivity() / camSample.pdf();

        Intersection isect;
        Color weight(1.0, 1.0, 1.0);
        Color throughput(0.0, 0.0, 0.0);
        for (int bounce = 0; ; bounce++) {
            const double rands[3] = { rstk.pop(), rstk.pop(), rstk.pop() };

            if (!scene.intersect(ray, isect) || bounce > params.bounceLimit()) {
                weight = Color::BLACK;
                pixel->weight = weight;
                pixel->coeff = coeff;
                pixel->emission += throughput;
                break;
            }

            const int       objectID = isect.objectId();
            const Hitpoint& hpoint   = isect.hitpoint();
            const BSDF&     bsdf     = scene.getBsdf(objectID);
            const Color&    emission = scene.isLightCheck(objectID) ? scene.directLight(ray.direction()) : Color::BLACK;
            const bool      into     = Vector3D::dot(hpoint.normal(),
                                                     ray.direction()) < 0.0;
            const Vector3D orientNormal = into ?  hpoint.normal()
                                               : -hpoint.normal();

            if (bsdf.type() & BsdfType::Lambertian) {
                // Ray hits diffuse object, return current weight
                pixel->position = hpoint.position();
                pixel->normal   = hpoint.normal();
                pixel->weight   = weight * bsdf.reflectance();
                pixel->coeff    = coeff;
                pixel->emission += throughput + weight * emission;
                break;
            } else {
                double pdf = 1.0;
                Vector3D nextdir;
                if (bsdf.type() & BsdfType::Bssrdf) {
                    Assertion(_integrator != NULL,
                              "Subsurface integrator is NULL !!");
                    Color bssrdfRad = bsdf.sampleBssrdf(ray.direction(),
                                                        hpoint.position(),
                                                        hpoint.normal(),
                                                        rands[1], rands[2],
                                                        *_integrator,
                                                        &nextdir, &pdf);
                    throughput += weight * bssrdfRad;
                } else {
                    bsdf.sample(ray.direction(), hpoint.normal(),
                                rands[1], rands[2], &nextdir, &pdf);
                }
                ray = Ray(hpoint.position(), nextdir);
                weight = weight * bsdf.reflectance() / pdf;
            }
        }
    }

}  // namespace spica
