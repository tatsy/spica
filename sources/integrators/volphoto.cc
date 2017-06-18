#define SPICA_API_EXPORT
#include "volphoto.h"

#include <cmath>
#include <ctime>
#include <cstdio>
#include <atomic>
#include <algorithm>
#include <mutex>

#include "../core/parallel.h"
#include "../core/memory.h"
#include "../core/kdtree.h"
#include "../core/interaction.h"
#include "../core/sampling.h"
#include "../core/renderparams.h"
#include "../scenes/scene.h"

#include "../integrator/photon_map.h"

#include "../bxdf/bsdf.h"
#include "../bxdf/bxdf.h"
#include "../bxdf/bssrdf.h"
#include "../bxdf/phase.h"

#include "mis.h"

namespace spica {

class VolPhotoIntegrator::VPhotonMap {
public:
    VPhotonMap()
        : kdtree_{} {
    }

    ~VPhotonMap() {
    }

    void clear() {
        kdtree_.release();
    }

    void construct(const Scene& scene,
                   const RenderParams& params) {
        std::cout << "Shooting photons..." << std::endl;

        // Compute light power distribution
        Distribution1D lightDistrib = mis::calcLightPowerDistrib(scene);

        // Random number generator
        const int nThreads = numSystemThreads();
        std::vector<std::unique_ptr<Sampler>> samplers(nThreads);
        for (int i = 0; i < nThreads; i++) {
            samplers[i] = std::make_unique<Random>((unsigned int)time(0) + i);
        }
        std::vector<MemoryArena> arenas(nThreads);

        // Distribute tasks
        const int castPhotons = params.get<int>("CAST_PHOTONS");
        std::vector<std::vector<Photon>> photons(nThreads);

        // Shooting photons
        std::atomic<int> proc(0);
        parallel_for(0, castPhotons, [&](int i) {
            const int threadID = getThreadID();
            const std::unique_ptr<Sampler>& sampler = samplers[threadID];
            sampler->startNextSample();

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
                                (lightPdf * pdfPos * pdfDir * castPhotons);
                if (!beta.isBlack()) {
                    tracePhoton(scene, params, photonRay, beta, *sampler,
                                arenas[threadID], &photons[threadID]);
                }
            }

            proc++;
            if (proc % 1000 == 0) {
                printf("%6.2f %% processed...\r", 
                       100.0 * proc / castPhotons);
            }
        });
        printf("\n");

        int numStored = 0;
        for (int i = 0; i < nThreads; i++) {
            numStored += photons[i].size();
        }
        printf("[INFO] %d photons stored.\n", numStored);

        // Contruct tree structure
        printf("Constructing photon map -> ");
        
        clear();
        std::vector<Photon> photonsAll;
        for (int i = 0; i < nThreads; i++) {
            photonsAll.insert(photonsAll.end(), 
                              photons[i].begin(), photons[i].end());
        }
        kdtree_.construct(photonsAll);

        printf("OK\n");
    }

    /**
     * Evaluate radiance at the point and to the direction.
     */
    Spectrum evaluateL(const SurfaceInteraction& po,
                      int gatherPhotons, double gatherRadius) const {
        // Find k-nearest neightbors
        Photon query(po.pos(), Spectrum(), po.wo(), po.normal());
        std::vector<Photon> photons;
        knnFind(query, &photons, gatherPhotons, gatherRadius);

        const int numPhotons = static_cast<int>(photons.size());

        // Extract valid photons using the normals
        std::vector<Photon> validPhotons;
        std::vector<double> distances;
        double maxdist = 0.0;
        for (int i = 0; i < numPhotons; i++) {
            const Vector3d diff = query.pos() - photons[i].pos();
            const double dist = diff.norm();
            const double dt   = vect::dot(po.normal(), diff) / dist;
            if (po.normal().norm() > EPS &&
                std::abs(dt) < gatherRadius * gatherRadius * 0.01) {
                validPhotons.push_back(photons[i]);
                distances.push_back(dist);
                maxdist = std::max(maxdist, dist);
            }
        }

        // Cone filter
        const int numValidPhotons = static_cast<int>(validPhotons.size());
        const double k = 1.1;
        Spectrum totalFlux = Spectrum(0.0, 0.0, 0.0);
        for (int i = 0; i < numValidPhotons; i++) {
            const double w = 1.0 - (distances[i] / (k * maxdist));
            const Spectrum v =
                validPhotons[i].beta() * po.bsdf()->f(po.wo(), validPhotons[i].wi());
            totalFlux += w * v;
        }
        totalFlux /= (1.0 - 2.0 / (3.0 * k));

        if (maxdist > EPS) {
            return Spectrum(totalFlux / (PI * maxdist * maxdist));
        }
        return Spectrum(0.0, 0.0, 0.0);    
    
    }

    Spectrum evaluateL(const MediumInteraction& mi,
                       int gatherPhotons, double gatherRadius) const {
        // Find k-nearest neightbors
        Photon query(mi.pos(), Spectrum(), mi.wo(), mi.normal());
        std::vector<Photon> photons;
        knnFind(query, &photons, gatherPhotons, gatherRadius);

        const int numPhotons = static_cast<int>(photons.size());

        // Extract valid photons using the normals
        std::vector<Photon> validPhotons;
        std::vector<double> distances;
        double maxdist = 0.0;
        for (int i = 0; i < numPhotons; i++) {
            if (photons[i].normal().norm() < EPS) {
                const Vector3d diff = query.pos() - photons[i].pos();
                const double dist = diff.norm();
    
                validPhotons.push_back(photons[i]);
                distances.push_back(dist);
                maxdist = std::max(maxdist, dist);
            }
        }
        if (validPhotons.empty()) return Spectrum(0.0);

        // Cone filter
        const int numValidPhotons = static_cast<int>(validPhotons.size());
        const double k = 1.1;
        Spectrum totalFlux = Spectrum(0.0, 0.0, 0.0);
        for (int i = 0; i < numValidPhotons; i++) {
            const double w = 1.0 - (distances[i] / (k * maxdist));
            const Spectrum v =
                validPhotons[i].beta() * mi.phase()->p(mi.wo(), validPhotons[i].wi());
                totalFlux += w * v;
        }
        totalFlux /= (1.0 - 3.0 / (4.0 * k));

        if (maxdist > EPS) {
            return Spectrum(totalFlux / ((4.0 / 3.0) * PI * maxdist * maxdist * maxdist));
        }
        return Spectrum(0.0, 0.0, 0.0);    
    }
        
private:
    void knnFind(const Photon& photon, std::vector<Photon>* photons, 
                    int gatherPhotons, double gatherRadius) const {
        KnnQuery query(K_NEAREST | EPSILON_BALL, gatherPhotons, gatherRadius);
        kdtree_.knnSearch(photon, query, photons);    
    }

    void tracePhoton(const Scene& scene,
                     const RenderParams& params,
                     const Ray& r,
                     const Spectrum& b,
                     Sampler& sampler,
                     MemoryArena& arena,
                     std::vector<Photon>* photons) {
        Ray ray(r);
        Spectrum beta(b);
        SurfaceInteraction isect;
        const int maxBounces = params.get<int>("MAX_BOUNCES");
        for (int bounces = 0; bounces < maxBounces; bounces++) {
            bool isIntersect = scene.intersect(ray, &isect);
        
            // Sample participating media
            MediumInteraction mi;
            Spectrum bnew;
            if (ray.medium()) bnew = beta * ray.medium()->sample(ray, sampler, arena, &mi);
            if (beta.isBlack()) break;

            if (mi.isValid()) {
                photons->emplace_back(mi.pos(), beta, -ray.dir(), Normal3d());
                
                Vector3d wo = -ray.dir();
                Vector3d wi;

                bnew = bnew * mi.phase()->sample(wo, &wi, sampler.get2D());
                double continueProb = std::min(1.0, bnew.luminance() / beta.luminance());
                if (sampler.get1D() > continueProb) break;
                beta = bnew / continueProb;

                ray = mi.spawnRay(wi);
            } else {
                if (!isIntersect) break;

                photons->emplace_back(isect.pos(), beta, -ray.dir(), isect.normal());

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
                    break;

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

private:
    KdTree<Photon> kdtree_;
};

VolPhotoIntegrator::VolPhotoIntegrator(
    const std::shared_ptr<const Camera>& camera,
    const std::shared_ptr<Sampler>& sampler,
    double alpha)
    : SamplerIntegrator{ camera, sampler }
    , photonmap_{}
    , globalRadius_{}
    , alpha_{ alpha } {
    photonmap_ = std::make_unique<VPhotonMap>();
}

VolPhotoIntegrator::~VolPhotoIntegrator() {
}

void VolPhotoIntegrator::initialize(const Scene& scene,
                                   const RenderParams& params,
                                   Sampler& sampler) {
    // Compute global radius
    Bounds3d bounds = scene.worldBound();
    globalRadius_ = (bounds.posMax() - bounds.posMin()).norm() * 0.5;
}

void VolPhotoIntegrator::loopStarted(const Scene& scene,
                                      const RenderParams& params,
                                      Sampler& sampler) {
    // Construct photon map
    photonmap_->construct(scene, params);
}

void VolPhotoIntegrator::loopFinished(const Scene& scene,
                                     const RenderParams& params,
                                     Sampler& sampler) {
    // Scale global radius
    globalRadius_ *= alpha_;   
}


Spectrum VolPhotoIntegrator::Li(const Scene& scene,
                               const RenderParams& params,
                               const Ray& r,
                               Sampler& sampler,
                               MemoryArena& arena,
                               int depth) const {
    Ray ray(r);
    Spectrum L(0.0);
    Spectrum beta(1.0);
    bool specularBounce = false;
    const int maxBounces      = params.get<int>("MAX_BOUNCES");
    const int gatherPhotons   = params.get<int>("GATHER_PHOTONS");
    const double gatherRadius = params.get<double>("GATHER_RADIUS");
    
    std::mutex mtx;
    for (int bounces = 0; ; bounces++) {
        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        // Sample participating media
        MediumInteraction mi;
        if (ray.medium()) {
            Spectrum Ld(0.0);
            for (;;) {
                beta *= ray.medium()->sample(ray, sampler, arena, &mi);
                if (beta.isBlack()) break;
                if (!mi.isValid()) break;

                Ld += beta * photonmap_->evaluateL(mi, gatherPhotons, gatherRadius);
                ray = Ray(mi.pos(), ray.dir(), INFTY, ray.medium());
                if (scene.intersect(ray, &isect)) {
                    break;
                }
            }
            L += Ld;
            break;
        } else {
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

            if (!isIntersect || bounces >= maxBounces) break;

            isect.setScatterFuncs(ray, arena);
            if (!isect.bsdf()) {
                ray = isect.spawnRay(ray.dir());
                bounces--;
                continue;
            }

            Spectrum Ld(0.0);
            if (isect.bsdf()->numComponents(BxDFType::All & (~BxDFType::Specular)) > 0) {
                Ld = beta * mis::uniformSampleOneLight(isect, scene, arena, sampler);
            }

            // Process BxDF
            Vector3d wo = -ray.dir();
            Vector3d wi;
            double pdf;
            BxDFType sampledType;
            Spectrum ref = isect.bsdf()->sample(wo, &wi, sampler.get2D(), &pdf,
                                                BxDFType::All, &sampledType);

            if (ref.isBlack() || pdf == 0.0) break;

            if ((sampledType & BxDFType::Diffuse) != BxDFType::None &&
                (sampledType & BxDFType::Reflection) != BxDFType::None) {
                L += beta * photonmap_->evaluateL(isect, gatherPhotons, 
                                                 gatherRadius);
                break;
            } else {
                L += Ld;
            }

            beta *= ref * vect::absDot(wi, isect.normal()) / pdf;
            specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
            ray = isect.spawnRay(wi);

            // Account for BSSRDF
            if (isect.bssrdf() && (sampledType & BxDFType::Transmission) != BxDFType::None) {
                SurfaceInteraction pi;
                Spectrum S = isect.bssrdf()->sample(scene, sampler.get1D(),
                    sampler.get2D(), arena, &pi, &pdf);

                if (S.isBlack() || pdf == 0.0) break;
                beta *= S / pdf;

                L += beta * mis::uniformSampleOneLight(pi, scene, arena, sampler);

                Spectrum f = pi.bsdf()->sample(pi.wo(), &wi, sampler.get2D(), &pdf,
                                               BxDFType::All, &sampledType);
                if (f.isBlack() || pdf == 0.0) break;
                beta *= f * vect::absDot(wi, pi.normal()) / pdf;

                specularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;
                ray = pi.spawnRay(wi);
            }
        }

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
