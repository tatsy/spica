#define SPICA_API_EXPORT
#include "photon_map.h"

#include <atomic>
#include <ctime>
#include <fstream>

#include "../core/memory.h"
#include "../core/parallel.h"
#include "../core/interaction.h"
#include "../core/sampling.h"
#include "../core/interaction.h"
#include "../core/renderparams.h"

#include "../scenes/scene.h"
#include "../camera/camera.h"
#include "../random/sampler.h"

#include "../bxdf/bxdf.h"
#include "../bxdf/bsdf.h"
#include "../bxdf/bssrdf.h"

#include "mis.h"

namespace spica {

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

double Photon::distance(const Photon& p1, const Photon& p2) {
    return (p1.pos_ - p2.pos_).norm();
}

PhotonMap::PhotonMap()
    : _kdtree{} {
}

PhotonMap::~PhotonMap() {
}

void PhotonMap::clear() {
    _kdtree.release();
}

void PhotonMap::construct(const Scene& scene,
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
    _kdtree.construct(photonsAll);
    printf("OK\n");
}

Spectrum PhotonMap::evaluateL(const SurfaceInteraction& po,
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
        if (std::abs(dt) < gatherRadius * gatherRadius * 0.01) {
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

Spectrum PhotonMap::evaluateE(const Interaction& intr,
                              int gatherPhotons, double gatherRadius) const {
    // Find k-nearest neightbors
    Photon query(intr.pos(), Spectrum(), Vector3d(), intr.normal());
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
        const double dt   = vect::dot(intr.normal(), diff) / dist;
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
        const Spectrum v =
            validPhotons[i].beta() * PI; // * vect::absDot(intr.normal(), validPhotons[i].wi());
        totalFlux += w * v;
    }
    totalFlux /= (1.0 - 2.0 / (3.0 * k));

    if (maxdist > EPS) {
        return Spectrum(totalFlux / (PI * maxdist * maxdist));
    }
    return Spectrum(0.0);
}

void PhotonMap::knnFind(const Photon& photon, std::vector<Photon>* photons,
                        const int gatherPhotons,
                        const double gatherRadius) const {
    KnnQuery query(K_NEAREST | EPSILON_BALL, gatherPhotons, gatherRadius);
    _kdtree.knnSearch(photon, query, photons);
}

void PhotonMap::tracePhoton(const Scene& scene,
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
        if (!scene.intersect(ray, &isect)) break;

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

}  // namespace spica
