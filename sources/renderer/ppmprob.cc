#define SPICA_API_EXPORT
#include "ppmprob.h"

#include <cmath>
#include <ctime>
#include <cstdio>
#include <algorithm>

#include "renderer_helper.h"
#include "render_parameters.h"
#include "subsurface_integrator.h"

#include "../image/tmo.h"

#include "../core/sampler.h"

#include "../random/random_sampler.h"
#include "../random/random.h"
#include "../random/halton.h"

#include "../light/lighting.h"

namespace spica {

    const double PPMPRenderer::kAlpha = 0.7;

    PPMPRenderer::PPMPRenderer()
        : IRenderer{RendererType::PhotonMap}
        , photonMap{}
        , globalRadius{0.0} {
    }

    PPMPRenderer::~PPMPRenderer() {
    }

    void PPMPRenderer::render(const Scene& scene, const Camera& camera, 
                            const RenderParameters& params) {
        const int width   = camera.imageW();
        const int height  = camera.imageH();
        const int samples = params.samplePerPixel();

        // Preparation for  accouting for BSSRDF
        _integrator->initialize(scene);

        // Random number generator
        RandomSampler* samplers = new RandomSampler[kNumThreads];
        for (int i = 0; i < kNumThreads; i++) {
            switch (params.randomType()) {
            case RandomType::MT19937:
                samplers[i] = Random::factory((unsigned int)i);
                break;

            case RandomType::Halton:
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
        globalRadius = (bbox.posMax() - bbox.posMin()).norm() * 0.5;

        // Distribute tasks
        const int tasksThread = (height + kNumThreads - 1) / kNumThreads;
        std::vector<std::vector<int> > tasks(kNumThreads);
        for (int y = 0; y < height; y++) {
            tasks[y % kNumThreads].push_back(y);
        }

        // Rendering
        Image buffer(width, height);
        buffer.fill(RGBSpectrum(0.0, 0.0, 0.0));
        _result.resize(width, height);
        for (int i = 0; i < params.samplePerPixel(); i++) {
            // Precomputation for subsurface scattering
            _integrator->construct(scene, params);

            // 1th pass: Construct photon map
            photonMap.construct(scene, params, BsdfType::Lambertian);

            // 2nd pass: Path tracing
            for (int t = 0; t < tasksThread; t++) {
                ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
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
            globalRadius = ((i + 1) + kAlpha) / ((i + 1) + 1.0) * globalRadius;

            // Buffer accumulation
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    _result.pixel(x, y) = buffer(x, y) / (i + 1);
                }
            }

            // Save image
            char filename[512];
            sprintf(filename, params.saveFilenameFormat().c_str(), i + 1);
            _result = GammaTmo(2.2).apply(_result);
            _result.save(filename);

            printf("save: %s\n (%d / %d)\n", filename, i + 1, samples);
        }
        printf("\nFinish!!\n");

        // Deallocate memories
        delete[] samplers;
    }

    // Path tracing
    Spectrum PPMPRenderer::tracePath(const Scene& scene,
                                     const Camera& camera,
                                     const RenderParameters& params, 
                                     Stack<double>& rstk, 
                                     const int pixelX,
                                     const int pixelY) const {
        CameraSample camSample = camera.sample(pixelX, pixelY, rstk);
        const Ray    ray       = camSample.ray();
        const double invpdf    = (camera.sensitivity() / camSample.pdf());

        return Spectrum(radiance(scene, params, ray, rstk, 0) * invpdf);
    }    

    // Recursive function to compute radiance
    Spectrum PPMPRenderer::radiance(const Scene& scene, 
                                    const RenderParameters& params, 
                                    const Ray& ray,
                                    Stack<double>& rseq,
                                    int bounces) const {
        // Too many bounces terminate recursion
        if (bounces >= params.bounceLimit()) {
            return Spectrum(0.0, 0.0, 0.0);
        }

        // Intersection test
        Intersection isect;
        if (!scene.intersect(ray, &isect)) {
            return scene.globalLight(ray.direction());
        }

        // Intersected object
        const int objID = isect.objectID();
        const BSDF& bsdf = scene.getBsdf(objID);

        const Spectrum emission = scene.isLightCheck(objID) ? scene.directLight(ray.direction()) : Spectrum(0.0, 0.0, 0.0);
        const bool into = vect::dot(ray.direction(), isect.normal()) < 0.0;
        const Normal orientNormal = into ?  isect.normal() 
                                         : -isect.normal();

        // Russian roulette
        const Spectrum& refl = isect.color();
        double roulette = max3(refl.red(), refl.green(), refl.blue());
        if (bounces > params.bounceStartRoulette()) {
            if (rseq.pop() > roulette) {
                return emission;
            }
        }
        roulette = 1.0;

        // Variables for next bounce
        Spectrum bssrdfRad(0.0, 0.0, 0.0);
        Vector3D nextdir;
        double pdf = 1.0;

        // Next radiance
        Spectrum nextRad(0.0, 0.0, 0.0);
        if (bsdf.type() & BsdfType::Lambertian) {
            nextRad = photonMap.evaluate(isect.position(),
                                         isect.normal(),
                                         params.gatherPhotons(),
                                         globalRadius);
        } else {
            bsdf.sample(ray.direction(), isect.normal(), 
                        rseq.pop(), rseq.pop(), &nextdir, &pdf);
            const Ray nextRay(isect.position(), nextdir);
            nextRad = radiance(scene, params, nextRay, rseq, bounces + 1);
        }

        // Account for BSSRDF
        if (bsdf.type() & BsdfType::Bssrdf) {
            double refPdf = 1.0;
            bssrdfRad = bsdf.evalBSSRDF(ray.direction(),
                                        isect.position(),
                                        isect.normal(),
                                        *_integrator,
                                        &refPdf);
            pdf *= refPdf;
        }
        return emission + (bssrdfRad + isect.color() * nextRad / pdf) / roulette;
    }

}  // namespace spica

