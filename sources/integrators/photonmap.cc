#define SPICA_API_EXPORT
#include "photonmap.h"

#include <atomic>
#include <ctime>
#include <fstream>

#include "core/memory.h"
#include "core/parallel.h"
#include "core/interaction.h"
#include "core/sampling.h"
#include "core/interaction.h"
#include "core/renderparams.h"

#include "core/scene.h"
#include "core/camera.h"
#include "core/sampler.h"

#include "core/bxdf.h"
#include "core/bsdf.h"
#include "core/bssrdf.h"
#include "core/phase.h"
#include "core/mis.h"

namespace spica {

namespace {

std::string typeString(PhotonMapType type) {
    if (type == PhotonMapType::Global) return "global";
    if (type == PhotonMapType::Caustics) return "caustics";
    if (type == PhotonMapType::Volumetric) return "volumetric";
    return "invalid";
}

}  // Anonymous namespace

Photon::Photon()
    : pos_{}
    , beta_{}
    , wi_{}
    , normal_{} {
}

Photon::Photon(const Point3d& pos, const Spectrum& beta, 
               const Vector3d& wi, const Normal3d& normal)
    : pos_{ pos }
    , beta_{ beta }
    , wi_{ wi }
    , normal_{ normal } {
}

Photon::Photon(const Photon& photon)
    : Photon{} {
    this->operator=(photon);
}

Photon::~Photon() {
}

Photon& Photon::operator=(const Photon& photon) {
    this->pos_    = photon.pos_;
    this->beta_   = photon.beta_;
    this->wi_     = photon.wi_;
    this->normal_ = photon.normal_;
    return *this;
}

double Photon::operator[](int id) const {
    return pos_[id];
}

double Photon::squaredDistance(const Photon& p1, const Photon& p2) {
    return (p1.pos_ - p2.pos_).squaredNorm();
}

PhotonMap::PhotonMap(PhotonMapType type)
    : type_{ type }
    , kdtree_{ } {
}

PhotonMap::~PhotonMap() {
}

void PhotonMap::clear() {
    kdtree_.release();
}

void PhotonMap::construct(const std::vector<Photon> &photons) {
    kdtree_.construct(photons);
}

void PhotonMap::construct(const Scene& scene,
                          RenderParams& params,
                          Sampler &sampler,
                          int photonCount) {

    MsgInfo("Shooting photons: %s", typeString(type_).c_str());

    // Compute light power distribution
    Distribution1D lightDistrib = calcLightPowerDistrib(scene);

    // Random number generator
    const int nThreads = numSystemThreads();
    std::vector<std::unique_ptr<Sampler>> samplers(nThreads);
    for (int i = 0; i < nThreads; i++) {
        samplers[i] = sampler.clone((uint32_t)time(0) + i);
    }
    std::vector<MemoryArena> arenas(nThreads);

    // Distribute tasks
    std::vector<std::vector<Photon>> photons(nThreads);

    // Shooting photons
    std::atomic<int> proc(0);
    parallel_for(0, photonCount, [&](int i) {
        const int threadID = getThreadID();
        const std::unique_ptr<Sampler>& sampler = samplers[threadID];
        sampler->startPixel();

        // Sample light source
        double lightPdf;
        const int lightID = std::min(lightDistrib.sampleDiscrete(sampler->get1D(), &lightPdf), lightDistrib.count() - 1);
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
                            (lightPdf * pdfPos * pdfDir * photonCount);
            if (!beta.isBlack()) {
                tracePhoton(scene, params, photonRay, beta, *sampler,
                            arenas[threadID], &photons[threadID]);
            }
        }

        proc++;
        if (proc % 1000 == 0) {
            printf("%6.2f %% processed...\r", 100.0 * proc / photonCount);
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

Spectrum PhotonMap::evaluateL(const SurfaceInteraction& po,
                              int gatherPhotons, double gatherRadius) const {
    // Find k-nearest neightbors
    Photon query(po.pos(), Spectrum(), po.wo(), po.ns());
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
        const double dt   = vect::dot(po.ns(), diff) / dist;
        if (std::abs(dt) < gatherRadius * gatherRadius * 0.01) {
            validPhotons.push_back(photons[i]);
            distances.push_back(dist);
            maxdist = std::max(maxdist, dist);
        }
    }

    // Cone filter
    const int numValidPhotons = static_cast<int>(validPhotons.size());
    const double k = 1.1;
    Spectrum totalFlux = Spectrum(0.0);
    for (int i = 0; i < numValidPhotons; i++) {
        const double w = 1.0 - (distances[i] / (k * maxdist));
        const Spectrum v = validPhotons[i].beta() * po.bsdf()->f(po.wo(), validPhotons[i].wi());
        totalFlux += w * v;
    }
    totalFlux /= (1.0 - 2.0 / (3.0 * k));

    Spectrum ret(0.0);
    if (maxdist > EPS) {
        ret = Spectrum(totalFlux / (PI * maxdist * maxdist));
    }
    return ret;
}

Spectrum PhotonMap::evaluateL(const MediumInteraction& mi,
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
        const Vector3d diff = query.pos() - photons[i].pos();
        const double dist = diff.norm();

        validPhotons.push_back(photons[i]);
        distances.push_back(dist);
        maxdist = std::max(maxdist, dist);
    }
    if (validPhotons.empty()) return Spectrum(0.0);

    // Cone filter
    const int numValidPhotons = static_cast<int>(validPhotons.size());
    const double k = 1.1;
    Spectrum totalFlux = Spectrum(0.0);
    for (int i = 0; i < numValidPhotons; i++) {
        const double w = 1.0 - (distances[i] / (k * maxdist));
        const Spectrum v = validPhotons[i].beta() * mi.phase()->p(mi.wo(), validPhotons[i].wi());
        totalFlux += w * v;
    }
    totalFlux /= (1.0 - 3.0 / (4.0 * k));

    Spectrum ret(0.0);
    if (maxdist > EPS) {
        Spectrum kappa = mi.getMedium()->sigmaExt(mi.pos());
        ret = (totalFlux / kappa) / ((4.0 / 3.0) * PI * maxdist * maxdist * maxdist);
    }
  
    return ret;
}

void PhotonMap::knnFind(const Photon& photon, std::vector<Photon>* photons,
                        const int gatherPhotons,
                        const double gatherRadius) const {
    KnnQuery query(K_NEAREST | EPSILON_BALL, gatherPhotons, gatherRadius);
    kdtree_.knnSearch(photon, query, photons);
}

void PhotonMap::tracePhoton(const Scene& scene,
                            RenderParams& params,
                            const Ray& r,
                            const Spectrum& b,
                            Sampler& sampler,
                            MemoryArena& arena,
                            std::vector<Photon>* photons) {
    Ray ray(r);
    Spectrum beta(b);
    SurfaceInteraction isect;
    bool specularBounce = false;
    const int maxBounces = params.getInt("maxDepth");
    for (int bounces = 0; ; bounces++) {
        bool isIntersect = scene.intersect(ray, &isect);
        
        // Sample participating media
        MediumInteraction mi;
        if (ray.medium()) beta *= ray.medium()->sample(ray, sampler, arena, &mi);
        if (beta.isBlack()) break;

        if (mi.isValid()) {
            if (bounces >= maxBounces) break;

            Vector3d wo = -ray.dir();
            Vector3d wi;
            Spectrum bnew = beta * mi.phase()->sample(wo, &wi, sampler.get2D());

            if (type_ == PhotonMapType::Volumetric) {
                photons->emplace_back(mi.pos(), beta, -ray.dir(), Normal3d(0.0, 0.0, 0.0));

                const double continueProb = std::min(1.0, bnew.gray() / beta.gray());
                if (sampler.get1D() > continueProb) break;

                bnew = bnew / continueProb;
            }

            ray = mi.spawnRay(wi);
            beta = bnew;

        } else {
            if (!isIntersect || bounces >= maxBounces) break;

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

            if (pdf < EPS || ref.isBlack()) break;

            Spectrum bnew = beta * ref * vect::absDot(wi, isect.ns()) / pdf;
            bool newSpecularBounce = (sampledType & BxDFType::Specular) != BxDFType::None;

            // A boolean "storeFlag" is true when each of conditions to store photons is satisfied.
            bool storeFlag = false;
            if ((sampledType & BxDFType::Diffuse) != BxDFType::None &&
                (sampledType & BxDFType::Reflection) != BxDFType::None) {
                // Diffuse or glossy reflection
                if (!specularBounce && type_ == PhotonMapType::Global) {
                    photons->emplace_back(isect.pos(), beta, -ray.dir(), isect.normal());
                    storeFlag = true;
                }

                if (specularBounce && type_ == PhotonMapType::Caustics) {
                    photons->emplace_back(isect.pos(), beta, -ray.dir(), isect.normal());
                    storeFlag = true;
                }
            }

            if (storeFlag) {
                const double continueProb = std::min(1.0, bnew.gray() / beta.gray());
                if (sampler.get1D() > continueProb) break;

                bnew = bnew / continueProb;
            }

            ray = isect.spawnRay(wi);
            beta = bnew;
            specularBounce = newSpecularBounce;

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
                beta *= f * vect::absDot(wi, pi.ns()) / pdf;

                ray = pi.spawnRay(wi);
            }
        }
    }
}

}  // namespace spica
