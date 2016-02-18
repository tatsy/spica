#define SPICA_API_EXPORT
#include "pathtrace.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "../core/common.h"
#include "../core/memory.h"
#include "../core/spectrum.h"
#include "../core/sampling.h"
#include "../core/interaction.h"

#include "../image/image.h"
#include "../image/tmo.h"

#include "../math/vector3d.h"

#include "../random/halton.h"
#include "../random/sampler.h"

#include "../bxdf/bsdf.h"
#include "../bxdf/bxdf.h"

#include "../scenes/scene.h"

#include "integrator.h"
#include "renderer_helper.h"
#include "render_parameters.h"
// #include "subsurface_integrator.h"

namespace spica {

PathRenderer::PathRenderer()
    : IRenderer{RendererType::PathTrace} {
}

PathRenderer::~PathRenderer() {
}

void PathRenderer::render(const Scene& scene, const Camera& camera,
                          const std::unique_ptr<Film>& film,
                          const RenderParameters& params) {
    const int width  = camera.imageW();
    const int height = camera.imageH();

    // Preparation for accouting for BSSRDF
    // _integrator->initialize(scene);

    // Prepare random number generators
    std::vector<Sampler> samplers;
    for (int i = 0; i < kNumThreads; i++) {
        switch (params.randomType()) {
        case RandomType::MT19937:
            samplers.push_back(Random::createSampler((unsigned int)time(0) + i));
            break;

        case RandomType::Halton:
            samplers.push_back(Halton::createSampler(300, true, (unsigned int)time(0) + i));
            break;

        default:
            FatalError("[ERROR] Unknown random number generator type!!");
        }
    }

    // Distribute rendering tasks
    const int taskPerThread = (height + kNumThreads - 1) / kNumThreads;
    std::vector<std::vector<int> > tasks(kNumThreads);
    for (int y = 0; y < height; y++) {
        tasks[y % kNumThreads].push_back(y);
    }

    // Trace rays
    int processed = 0;
    for (int i = 0; i < params.samplePerPixel(); i++) {
        if (i % kNumThreads == 0) {
            // _integrator->construct(scene, params);
        }

        for (int t = 0; t < taskPerThread; t++) {
            ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
                if (t < tasks[threadID].size()) {
                    Stack<double> rstk;
                    const int y = tasks[threadID][t];
                    for (int x = 0; x < width; x++) {
                        film->addPixel(width - x - 1, y, tracePath(scene, camera, params, x, y, samplers[threadID]));
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

Spectrum PathRenderer::tracePath(const Scene& scene, const Camera& camera, 
                                    const RenderParameters& params,
                                    const double pixelX, const double pixelY,
                                    Sampler& sampler) {
    CameraSample camSample = camera.sample(pixelX, pixelY, sampler.get2D());
    
    const Ray ray = camSample.ray();
    return radiance(scene, params, ray, sampler, 0) * (camera.sensitivity() / camSample.pdf());
}

Spectrum PathRenderer::Li(const Scene& scene,
                          const RenderParameters& params,
                          const Ray& r,
                          Sampler& sampler,
                          int depth) const {
    MemoryArena arena;
    Ray ray(r);
    Spectrum L(0.0);
    Spectrum beta(1.0);
    bool specularBounce = false;
    int bounces;
    for (bounces = 0; ; bounces++) {
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

        if (!isIntersect || bounces >= params.bounceLimit()) break;

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
        Vector3D wo = -ray.dir();
        Vector3D wi;
        double pdf;
        BxDFType sampledType;
        Spectrum ref = isect.bsdf()->sample(wo, &wi, sampler.get2D(), &pdf,
                                            BxDFType::All, &sampledType);

        if (ref.isBlack() || pdf == 0.0) break;

        beta *= ref * vect::absDot(wi, isect.normal()) / pdf;
        specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
        ray = isect.spawnRay(wi);

        // Account for BSSRDF
        /*
        if (bsdf.type() & BsdfType::Bssrdf) {
            Assertion(_integrator != nullptr,
                        "Subsurface intergrator is NULL !!");

            double refPdf = 1.0;
            bssrdfRad = bsdf.evalBSSRDF(ray.dir(),
                                        isect.pos(),
                                        isect.normal(),
                                        *_integrator,
                                        &refPdf);
            pdf *= refPdf;
        }
        */

        // Russian roulette
        if (bounces > 3) {
            double continueProbability = std::min(0.95, beta.luminance());
            if (sampler.get1D() > continueProbability) break;
            beta /= continueProbability;
        }
    }
    return L;
}

}  // namespace spica
