#define SPICA_API_EXPORT
#include "pathtrace.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "../core/common.h"
#include "../core/sampler.h"
#include "../image/image.h"
#include "../image/tmo.h"

#include "../math/vector3d.h"

#include "../random/halton.h"
#include "../random/random_sampler.h"

#include "../scenes/scene.h"
#include "../light/lighting.h"

#include "renderer_helper.h"
#include "render_parameters.h"
#include "subsurface_integrator.h"

namespace spica {

    PathRenderer::PathRenderer()
        : IRenderer{RendererType::PathTrace} {
    }

    PathRenderer::~PathRenderer() {
    }

    void PathRenderer::render(const Scene& scene, const Camera& camera,
                              const RenderParameters& params) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        // Preparation for accouting for BSSRDF
        _integrator->initialize(scene);

        // Prepare random number generators
        auto samplers = std::vector<RandomSampler>(kNumThreads);
        for (int i = 0; i < kNumThreads; i++) {
            switch (params.randomType()) {
            case RandomType::MT19937:
                samplers[i] = RandomSampler::useMersenne((unsigned int)time(0) + i);
                break;

            case RandomType::Halton:
                samplers[i] = RandomSampler::useHalton(300, true, (unsigned int)time(0) + i);
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
        const int taskPerThread = (height + kNumThreads - 1) / kNumThreads;
        std::vector<std::vector<int> > tasks(kNumThreads);
        for (int y = 0; y < height; y++) {
            tasks[y % kNumThreads].push_back(y);
        }

        // Trace rays
        int processed = 0;
        _result.resize(width, height);
        buffer.fill(Color::BLACK);
        for (int i = 0; i < params.samplePerPixel(); i++) {
            if (i % kNumThreads == 0) {
                _integrator->construct(scene, params);
            }

            for (int t = 0; t < taskPerThread; t++) {
                ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
                    if (t < tasks[threadID].size()) {
                        Stack<double> rstk;
                        const int y = tasks[threadID][t];
                        for (int x = 0; x < width; x++) {
                            samplers[threadID].request(&rstk, 300);
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
            _result = GammaTmo(2.2).apply(_result);
            _result.save(filename);

            printf("%6.2f %%  processed -> %s\r",
                   100.0 * (i + 1) / params.samplePerPixel(), filename);
        }
        printf("\nFinish!!\n");
    }

    Color PathRenderer::tracePath(const Scene& scene, const Camera& camera, 
                                  const RenderParameters& params,
                                  const double pixelX, const double pixelY,
                                  Stack<double>& rands) {
        CameraSample camSample = camera.sample(pixelX, pixelY, rands);
        const Ray ray = camSample.ray();

        return radiance(scene, params, ray, rands, 0) * (camera.sensitivity() / camSample.pdf());
    }

    Color PathRenderer::radiance(const Scene& scene,
                                const RenderParameters& params,
                                const Ray& ray, Stack<double>& rstack,
                                int bounces) const {
        if (bounces >= params.bounceLimit()) {
            return Color::BLACK;
        }

        Intersection isect;
        if (!scene.intersect(ray, &isect)) {
            return scene.globalLight(ray.direction());
        }

        // Get intersecting material
        const int objectID     = isect.objectID();
        const BSDF& bsdf       = scene.getBsdf(objectID);
        const Color& refl      = isect.color();

        // Russian roulette
        double roulette = max3(refl.red(), refl.green(), refl.blue());
        if (bounces < params.bounceStartRoulette()) {
            roulette = 1.0;
        } else {
            if (roulette <= rstack.pop()) {
                return Color::BLACK;
            }
        }

        // Variables for next bounce
        Color bssrdfRad(0.0, 0.0, 0.0);
        Vector3D nextdir;
        double pdf = 1.0;

        // Sample next direction
        bsdf.sample(ray.direction(), isect.normal(), 
                    rstack.pop(), rstack.pop(), &nextdir, &pdf);

        // Account for BSSRDF
        if (bsdf.type() & BsdfType::Bssrdf) {
            Assertion(_integrator != nullptr,
                      "Subsurface intergrator is NULL !!");

            double refPdf = 1.0;
            bssrdfRad = bsdf.evalBSSRDF(ray.direction(),
                                        isect.position(),
                                        isect.normal(),
                                        *_integrator,
                                        &refPdf);
            pdf *= refPdf;
        }

        // Sample direct lighting
        Color directrad = directSample(scene, objectID, ray.direction(),
                                       isect.position(), isect.normal(),
                                       refl, bounces, rstack);

        // Compute next bounce
        const Ray nextray(isect.position(), nextdir);
        const Color nextrad = radiance(scene, params, nextray,
                                       rstack, bounces + 1);            


        return (bssrdfRad + directrad + refl * nextrad / pdf) / roulette;
    }

    double powerHeuristic(int nf, double f, int ng, double g) {
        const double ff = nf * f;
        const double gg = ng * g;
        return (ff * ff) / (ff * ff + gg * gg);
    }

    Color PathRenderer::directSample(const Scene& scene, const int triID,
                                     const Vector3D& in, const Vector3D& v,
                                     const Vector3D& n, const Color& refl,
                                     int bounces, Stack<double>& rstk) const {
        const BSDF& bsdf = scene.getBsdf(triID);
        if (bsdf.type() & BsdfType::Scatter) {
            // Scattering surface
            if (bounces == 0 && scene.isLightCheck(triID)) {
                return scene.directLight(in);
            } else {
                Color Ld(0.0, 0.0, 0.0);

                // Sample light with multiple importance sampling
                const LightSample Ls = scene.sampleLight(v, rstk);                
                if (Ls.pdf() != 0.0) {
                    // Visibility check
                    Intersection isect;
                    if (scene.intersect(Ray(v, -Ls.dir()), &isect)) {
                        if (scene.isLightCheck(isect.objectID())) {
                            // PDFs are computed for polar coordinate system
                            double lightPdf = Ls.pdf();
                            double bsdfPdf  = bsdf.pdf(in, n, -Ls.dir());

                            double weight = powerHeuristic(1, lightPdf, 1, bsdfPdf);

                            double dot = std::abs(Vector3D::dot(n, -Ls.dir()));
                            Ld += (refl * dot * Ls.Le()) * weight / lightPdf; 
                        }
                    }
                }

                // Sample BSDF with multiple importance sampling (only for area lights)
                Vector3D nextdir;
                double bsdfPdf;
                bsdf.sample(in, n, rstk.pop(), rstk.pop(), &nextdir, &bsdfPdf);

                Ray sampleRay(v, nextdir);
                Intersection lightIsect;
                if (scene.intersect(sampleRay, &lightIsect)) {
                    if (scene.isLightCheck(lightIsect.objectID())) {
                        const double dist2 = (lightIsect.position() - v).squaredNorm();
                        const double dot0  = Vector3D::dot(n, nextdir);
                        const double dot1  = Vector3D::dot(lightIsect.normal(), -nextdir);
                        if (dot0 > EPS && dot1 > EPS) {
                            // PDFs are computed for polar coordinate system
                            double jacob = dot1 / dist2;
                            double lightPdf = 1.0 / (INV_PI * jacob * scene.lightArea());

                            double weight = powerHeuristic(1, bsdfPdf, 1, lightPdf);

                            Ld += (refl * dot0 * scene.directLight(nextdir)) * weight / bsdfPdf; 
                        }
                    }
                }
                return Ld;
            }
        } else if (bsdf.type() & BsdfType::Dielectric) {
            // Dielectric surface
            double pdf;
            Vector3D nextdir;
            bsdf.sample(in, n, rstk.pop(), rstk.pop(), &nextdir, &pdf);
            
            Intersection isect;
            if (scene.intersect(Ray(v, nextdir), &isect)) {
                if (scene.isLightCheck(isect.objectID())) {
                    return (refl * scene.directLight(nextdir)) / pdf;
                }
            }
        } else {
            SpicaError("Invalid BSDF detected: this is "
                       "neigher scattering nor dielectric!!");
        }
        return Color(0.0, 0.0, 0.0);
    }

}  // namespace spica
