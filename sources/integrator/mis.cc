#define SPICA_API_EXPORT
#include "mis.h"

#include "../core/timer.h"
#include "../core/memory.h"
#include "../core/interaction.h"
#include "../core/ray.h"
#include "../core/point2d.h"
#include "../core/sampling.h"

#include "../camera/camera.h"
#include "../scenes/scene.h"
#include "../random/sampler.h"
#include "../bxdf/bsdf.h"
#include "../bxdf/bxdf.h"
#include "../bxdf/phase.h"
#include "../shape/visibility_tester.h"

namespace spica {

namespace mis {

Spectrum uniformSampleOneLight(const Interaction& intr, const Scene& scene,
                               MemoryArena& arena, Sampler& sampler,
                               bool handleMedia) {
    const int nLights = static_cast<int>(scene.lights().size());
    if (nLights == 0) return Spectrum(0.0);

    const int lightID = std::min((int)(sampler.get1D() * nLights), nLights - 1);
    const auto& light = scene.lights()[lightID];
    const Point2d randLight = sampler.get2D();
    const Point2d randShade = sampler.get2D();
    return nLights * estimateDirectLight(intr, randShade, *light, randLight,
                                         scene, sampler, arena);
}

Spectrum estimateDirectLight(const Interaction& intr,
                             const Point2d& randShade,
                             const Light& light,
                             const Point2d& randLight,
                             const Scene& scene, Sampler& sampler,
                             MemoryArena& arena,
                             bool specular,
                             bool handleMedia) {
    BxDFType bxdfType = specular ? BxDFType::All : (BxDFType::All & (~BxDFType::Specular));

    Spectrum Ld(0.0);

    // Sample light with multiple importance sampling
    Vector3d wi;
    VisibilityTester vis;
    double lightPdf = 0.0, bsdfPdf = 0.0;
    Spectrum Li = light.sampleLi(intr, randLight, &wi, &lightPdf, &vis);
    if (lightPdf > 0.0 && !Li.isBlack()) {
        Spectrum f;
        if (intr.isSurfaceInteraction()) {
            const SurfaceInteraction& isect =
                static_cast<const SurfaceInteraction&>(intr);
            f = isect.bsdf()->f(isect.wo(), wi, bxdfType) *
                vect::absDot(wi, isect.normal());
            bsdfPdf = isect.bsdf()->pdf(isect.wo(), wi, bxdfType);
        } else {
            // Evaluate phase function
            const MediumInteraction& mi =
                static_cast<const MediumInteraction&>(intr);
            double p = mi.phase()->p(mi.wo(), wi);
            f = Spectrum(p);
            bsdfPdf = p;
        }

        if (!f.isBlack()) {
            if (handleMedia) {
                Li *= vis.transmittance(scene, sampler);
            } else if (!vis.unoccluded(scene)) {
                Li = Spectrum(0.0);
            }

            if (!Li.isBlack()) {
                if (light.isDelta()) {
                    Ld += f * Li / lightPdf;
                } else {
                    const double weight = powerHeuristic(1, lightPdf, 1, bsdfPdf);
                    Ld += f * Li * weight / lightPdf;
                }
            }
        }
    }

    // Sample BSDF with multiple importance sampling
    if (!light.isDelta()) {
        Spectrum f;
        bool sampledSpecular = false;
        if (intr.isSurfaceInteraction()) {
            BxDFType sampledType;
            const auto& isect = static_cast<const SurfaceInteraction&>(intr);
            f = isect.bsdf()->sample(isect.wo(), &wi, randShade, &bsdfPdf, bxdfType, &sampledType);
            f *= vect::absDot(wi, isect.normal());
            sampledSpecular = (sampledType & BxDFType::Specular) != BxDFType::None;
        } else {
            // Sample new direction after scattered with participating media
            const MediumInteraction& mi = 
                static_cast<const MediumInteraction&>(intr);
            double p = mi.phase()->sample(mi.wo(), &wi, randShade);
            f = Spectrum(p);
            bsdfPdf = p;
        }

        if (!f.isBlack() && bsdfPdf > 0.0) {
            double weight = 1.0;
            if (!sampledSpecular) {
                lightPdf = light.pdfLi(intr, wi);
                if (lightPdf == 0.0) return Ld;
                weight = powerHeuristic(1, bsdfPdf, 1, lightPdf);
            }

            SurfaceInteraction lightIsect;
            Ray ray = intr.spawnRay(wi);
            Spectrum Tr(1.0);
            bool foundSurfaceInteraction = 
                handleMedia ? scene.intersectTr(ray, sampler, &lightIsect, &Tr)
                            : scene.intersect(ray, &lightIsect); 

            Spectrum Li(0.0);
            if (foundSurfaceInteraction) {
                if (lightIsect.primitive()->areaLight() == reinterpret_cast<const AreaLight*>(&light)) {
                    Li = lightIsect.Le(-wi);   
                }
            } else {
                Li = light.Le(ray);
            }

            if (!Li.isBlack()) {
                Ld += f * Li * Tr * weight / bsdfPdf;
            }
        }
    }
    return Ld;

}

double powerHeuristic(int nf, double fPdf, int ng, double gPdf) {
    double f = nf * fPdf, g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}

Distribution1D calcLightPowerDistrib(const Scene& scene) {
    if (scene.lights().size() == 0) return Distribution1D{};

    std::vector<double> powers;
    for (const auto& light : scene.lights()) {
        powers.push_back(light->power().luminance());
    }
    return Distribution1D(powers);
}

}  // namespace mis

}  // namespace spica