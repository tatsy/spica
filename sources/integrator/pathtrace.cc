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
    buffer.fill(RGBSpectrum(0.0, 0.0, 0.0));
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
                        buffer.pixel(width - x - 1, y) +=
                            tracePath(scene, camera, params, x, y, samplers[threadID]);
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

Spectrum PathRenderer::radiance(const Scene& scene,
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

// ----------------------------------------------------------------------------

    /*
    if (bounces >= params.bounceLimit()) {
        return RGBSpectrum(0.0);
    }

    SurfaceInteraction intr;
    //if (!scene.intersect(ray, &intr)) {
    //    return Spectrum(0.0);
    //}

    // Get intersecting material
    const BSDF& bsdf       = intr.bsdf();
    const Spectrum& refl   = intr.color();

    // Russian roulette
    double roulette = max3(refl.red(), refl.green(), refl.blue());
    if (bounces < params.bounceStartRoulette()) {
        roulette = 1.0;
    } else {
        if (roulette <= rstack.pop()) {
            return Spectrum(0.0, 0.0, 0.0);
        }
    }

    // Variables for next bounce
    Spectrum bssrdfRad(0.0, 0.0, 0.0);
    Vector3D nextdir;
    double pdf = 1.0;

    // Sample next direction
    bsdf.sample(ray.dir(), isect.normal(), 
                rstack.pop(), rstack.pop(), &nextdir, &pdf);

    // Account for BSSRDF
    if (bsdf.type() & BsdfType::Bssrdf) {
        Assertion(_integrator != nullptr,
                    "Subsurface intergrator is NULL !!");

        double refPdf = 1.0;
        bssrdfRad = bsdf.evalBSSRDF(ray.dir(),
                                    isect.position(),
                                    isect.normal(),
                                    *_integrator,
                                    &refPdf);
        pdf *= refPdf;
    }

    // Sample direct lighting
    Spectrum directrad = directSample(scene, objectID, ray.dir(),
                                        isect.position(), isect.normal(),
                                        refl, bounces, rstack);

    // Compute next bounce
    const Ray nextray(isect.position(), nextdir);
    const Spectrum nextrad = radiance(scene, params, nextray,
                                        rstack, bounces + 1);            

    return (bssrdfRad + directrad + refl * nextrad / pdf) / roulette;
    */
}

/*
Spectrum PathRenderer::directSample(const Scene& scene, const int triID,
                                    const Vector3D& in, const Point& v,
                                    const Normal& n, const Spectrum& refl,
                                    int bounces, Stack<double>& rstk) const {
    const BSDF& bsdf = scene.getBsdf(triID);
    if (bsdf.type() & BsdfType::Scatter) {
        // Scattering surface
        if (bounces == 0 && scene.isLightCheck(triID)) {
            return scene.directLight(in);
        } else {
            Spectrum Ld(0.0, 0.0, 0.0);

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

                        double dot = std::abs(vect::dot(n, -Ls.dir()));
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
                    const double dot0  = vect::dot(n, nextdir);
                    const double dot1  = vect::dot(lightIsect.normal(), -nextdir);
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
        FatalError("Invalid BSDF detected: this is "
                    "neigher scattering nor dielectric!!");
    }
    return Spectrum(0.0, 0.0, 0.0);
}
*/

}  // namespace spica
