#define SPICA_PT_RENDERER_EXPORT
#include "pathtrace.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "../utils/common.h"
#include "../utils/vector3d.h"
#include "../utils/sampler.h"
#include "../utils/image.h"

#include "../random/random_sampler.h"

#include "scene.h"
#include "renderer_helper.h"
#include "subsurface_integrator.h"

namespace spica {

    PathRenderer::PathRenderer()
        : IRenderer()
    {
    }

    PathRenderer::~PathRenderer()
    {
    }

    void PathRenderer::render(const Scene& scene, const Camera& camera,
                                     const RenderParameters& params) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        // Preparation for accouting for BSSRDF
        _integrator->initialize(scene, params);

        // Prepare random number generators
        RandomSampler* samplers = new RandomSampler[kNumCores];
        for (int i = 0; i < kNumCores; i++) {
            switch (params.randomType()) {
            case PSEUDO_RANDOM_TWISTER:
                samplers[i] = Random::factory(i);
                break;

            case QUASI_MONTE_CARLO:
                samplers[i] = Halton::factory(200, true, i);
                break;

            default:
                std::cerr << "[ERROR] Unknown random number generator type!!"
                          << std::endl;
                std::abort();
            }
        }

        // Vectors spanning screen
        Image buffer = Image(width, height);

        // Distribute rendering tasks
        const int taskPerThread = (height + kNumCores - 1) / kNumCores;
        std::vector<std::vector<int> > tasks(kNumCores);
        for (int y = 0; y < height; y++) {
            tasks[y % kNumCores].push_back(y);
        }

        // Trace rays
        int processed = 0;
        _result.resize(width, height);
        buffer.fill(Color::BLACK);
        for (int i = 0; i < params.samplePerPixel(); i++) {
            for (int t = 0; t < taskPerThread; t++) {
                ompfor (int threadID = 0; threadID < kNumCores; threadID++) {
                    if (t < tasks[threadID].size()) {
                        Stack<double> rstk;
                        const int y = tasks[threadID][t];
                        for (int x = 0; x < width; x++) {
                            samplers[threadID].request(&rstk, 200);
                            buffer.pixel(width - x - 1, y) +=
                                tracePath(scene, camera, params, x, y, rstk);
                        }
                    }
                }
            }

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    _result.pixel(x, y) = buffer(x, y) / (i + 1);
                }
            }

            char filename[256];
            sprintf(filename, params.saveFilenameFormat().c_str(), i + 1);
            _result.gammaCorrect(1.0 / 2.2);
            _result.save(filename);

            printf("%6.2f %%  processed -> %s\r",
                    100.0 * (i + 1) / params.samplePerPixel(), filename);
        }
        printf("\nFinish!!\n");

        delete[] samplers;
    }

    Color PathRenderer::tracePath(const Scene& scene, const Camera& camera, 
                                  const RenderParameters& params,
                                  const double pixelX, const double pixelY,
                                  Stack<double>& rands) {
        CameraSample camSample = camera.sample(pixelX, pixelY, rands);
        const Ray ray = camSample.generateRay();

        return Color(radiance(scene, params, ray, rands, 0) * 
                        (camera.sensitivity() / camSample.totalPdf()));
    }

    Color PathRenderer::radiance(const Scene& scene,
                                const RenderParameters& params,
                                const Ray& ray, Stack<double>& rstack,
                                int bounces) const {
        if (bounces >= params.bounceLimit()) {
            return Color::BLACK;
        }

        Intersection isect;
        if (!scene.intersect(ray, isect)) {
            return scene.envmap().sampleFromDir(ray.direction());
        }

        // Require random numbers
        const double randnums[3] = { rstack.pop(), rstack.pop(), rstack.pop() };

        // Get intersecting material
        const int objectID     = isect.objectId();
        const BSDF& bsdf       = scene.getBsdf(objectID);
        const Color& refl      = bsdf.reflectance();
        const Color& emittance = scene.getEmittance(objectID);
        const Hitpoint& hpoint = isect.hitpoint();

        // Russian roulette
        double roulette = max3(refl.red(), refl.green(), refl.blue());
        if (bounces < params.bounceStartRoulette()) {
            roulette = 1.0;
        } else {
            if (roulette <= randnums[0]) {
                return emittance;
            }
        }

        // Variables for next bounce
        Color bssrdfRad(0.0, 0.0, 0.0);
        Vector3D nextdir;
        double pdf = 1.0;

        // Account for BSSRDF
        if (bsdf.type() & BSDF_TYPE_BSSRDF) {
            Assertion(_integrator != NULL,
                      "Subsurface intergrator is NULL !!");
            bssrdfRad = bsdf.sampleBssrdf(ray.direction(),
                                          hpoint.position(),
                                          hpoint.normal(),
                                          randnums[1], randnums[2],
                                          *_integrator,
                                          &nextdir, &pdf);
        } else {
            // Sample next direction
            bsdf.sample(ray.direction(), hpoint.normal(), 
                        randnums[1], randnums[2], &nextdir, &pdf);
            
        }

        // Compute next bounce
        const Ray nextray(hpoint.position(), nextdir);
        const Color nextrad = radiance(scene, params, nextray,
                                       rstack, bounces + 1);
            
        // Return result
        return Color(emittance + (bssrdfRad + refl * nextrad / pdf) / roulette);
    }

}  // namespace spica
