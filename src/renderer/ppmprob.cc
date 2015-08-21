#define SPICA_PHOTON_MAPPING_EXPORT
#include "ppmprob.h"

#include <cmath>
#include <ctime>
#include <cstdio>
#include <algorithm>

#include "renderer_helper.h"
#include "../utils/sampler.h"
#include "../random/random_sampler.h"

namespace spica {

    const double PPMPRenderer::kAlpha = 0.7;

    PPMPRenderer::PPMPRenderer()
        : IRenderer()
        , photonMap()
        , globalRadius(0.0)
    {
    }

    PPMPRenderer::~PPMPRenderer()
    {
    }

    void PPMPRenderer::render(const Scene& scene, const Camera& camera, 
                            const RenderParameters& params) {
        const int width   = camera.imageW();
        const int height  = camera.imageH();
        const int samples = params.samplePerPixel();

        // Random number generator
        RandomSampler* samplers = new RandomSampler[kNumCores];
        for (int i = 0; i < kNumCores; i++) {
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
            }
        }

        // Compute global radius for PPMP
        BBox bbox;
        for (int i = 0;  i < scene.numTriangles(); i++) {
            bbox.merge(scene.getTriangle(i));
        }
        globalRadius = (bbox.posMax() - bbox.posMin()).norm() * 0.1;
        RenderParameters globalParams = params;
        globalParams.gatherRadius(globalRadius);

        // Distribute tasks
        const int tasksThread = (height + kNumCores - 1) / kNumCores;
        std::vector<std::vector<int> > tasks(kNumCores);
        for (int y = 0; y < height; y++) {
            tasks[y % kNumCores].push_back(y);
        }

        // Rendering
        Image buffer(width, height);
        buffer.fill(Color::BLACK);
        _result.resize(width, height);
        for (int i = 0; i < params.samplePerPixel(); i++) {
            // 1th pass: Construct photon map
            photonMap.construct(scene, camera, params);

            // 2nd pass: Path tracing
            for (int t = 0; t < tasksThread; t++) {
                ompfor (int threadID = 0; threadID < kNumCores; threadID++) {
                    if (t < tasks[threadID].size()) {
                        Stack<double> rstk;            
                        const int y = tasks[threadID][t];
                        for (int x = 0; x < width; x++) {
                            samplers[threadID].request(&rstk, 250);
                            buffer.pixel(width - x - 1, y) += 
                                tracePath(scene, camera, params, rstk, x, y);
                        }
                    }
                }
                printf("%6.2f %%  processed ... \r",
                       100.0 * (t + 1) / tasksThread);
            }
            printf("\n");

            // Update gather radius
            globalRadius = ((i + 1) + 1) / ((i + 1) + kAlpha) * globalRadius;
            globalParams.gatherRadius(globalRadius);

            // Buffer accumulation
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    _result.pixel(x, y) = buffer(x, y) / (i + 1);
                }
            }

            // Save image
            char filename[512];
            sprintf(filename, params.saveFilenameFormat().c_str(), i + 1);
            _result.gammaCorrect(1.0 / 2.2);
            _result.save(filename);

            printf("save: %s\n (%d / %d)", filename, i + 1, samples);
        }
        printf("\nFinish!!\n");

        // Deallocate memories
        delete[] samplers;
    }

    // Path tracing
    Color PPMPRenderer::tracePath(const Scene& scene,
                                  const Camera& camera,
                                  const RenderParameters& params, 
                                  Stack<double>& rstk, 
                                  const int pixelX,
                                  const int pixelY) const {
        CameraSample camSample = camera.sample(pixelX, pixelY, rstk);
        const Ray    ray       = camSample.generateRay();
        const double invpdf    = (camera.sensitivity() / camSample.totalPdf());

        return Color(radiance(scene, params, ray, rstk, 0) * invpdf);
    }    

    // Recursive function to compute radiance
    Color PPMPRenderer::radiance(const Scene& scene, 
                                 const RenderParameters& params, 
                                 const Ray& ray,
                                 Stack<double>& rseq,
                                 int bounces) const {
        // Too many bounces terminate recursion
        if (bounces >= params.bounceLimit()) {
            return Color::BLACK;
        }

        // Intersection test
        Intersection isect;
        if (!scene.intersect(ray, isect)) {
            return scene.envmap().sampleFromDir(ray.direction());
        }

        // Request random numbers
        const double rands[3] = { rseq.pop(), rseq.pop(), rseq.pop() };

        // Intersected object
        const int objID = isect.objectId();
        const BSDF& bsdf = scene.getBsdf(objID);
        const Color& emission = scene.getEmittance(objID);
        const Hitpoint hpoint = isect.hitpoint();
        const bool into = Vector3D::dot(ray.direction(), hpoint.normal()) < 0.0;
        const Vector3D orientNormal = into ?  hpoint.normal() 
                                           : -hpoint.normal();

        // Russian roulette
        const Color& refl = bsdf.reflectance();
        double roulette = max3(refl.red(), refl.green(), refl.blue());
        if (bounces > params.bounceStartRoulette()) {
            if (rands[0] > roulette) {
                return emission;
            }
        }
        roulette = 1.0;

        // Reflection
        Color nextRad(0.0, 0.0, 0.0);
        if (bsdf.type() & BSDF_TYPE_LAMBERTIAN_BRDF) {
            nextRad = photonMap.evaluate(hpoint.position(),
                                         ray.direction(), hpoint.normal(),
                                         params.gatherPhotons(),
                                         params.gatherRadius());
        } else {
            double pdf = 1.0;
            Vector3D nextDir;
            bsdf.sample(ray.direction(), hpoint.normal(), 
                        rands[1], rands[2], &nextDir, &pdf);
            Ray nextRay = Ray(hpoint.position(), nextDir);
            nextRad = radiance(scene, params, nextRay, rseq, bounces + 1) / pdf;
        }

        return Color(emission + (bsdf.reflectance() * nextRad) / roulette);
    }

}  // namespace spica
