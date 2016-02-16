#define SPICA_API_EXPORT
#include "photon_map.h"

#include <ctime>
#include <fstream>

#include "../core/sampling.h"
#include "../scenes/scene.h"
#include "../camera/camera.h"
#include "../random/sampler.h"
#include "../random/random.h"
#include "../random/halton.h"

#include "render_parameters.h"

namespace spica {

    Photon::Photon()
        : _position{}
        , _flux{}
        , _direction{}
        , _normal{} {
    }

    Photon::Photon(const Point& position, const Spectrum& flux, 
                   const Vector3D& direction, const Normal& normal)
        : _position{position}
        , _flux{flux}
        , _direction{direction}
        , _normal{normal} {
    }

    Photon::Photon(const Photon& photon)
        : Photon{} {
        this->operator=(photon);
    }

    Photon::~Photon() {
    }

    Photon& Photon::operator=(const Photon& photon) {
        this->_position  = photon._position;
        this->_flux      = photon._flux;
        this->_direction = photon._direction;
        this->_normal    = photon._normal;
        return *this;
    }

    double Photon::get(int id) const {
        return _position[id];
    }

    double Photon::distance(const Photon& p1, const Photon& p2) {
        return (p1._position - p2._position).norm();
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
                              const RenderParameters& params,
                              BsdfType absorbBsdf) {

        std::cout << "Shooting photons..." << std::endl;

        // Random number generator
        RandomSampler* samplers = new RandomSampler[kNumThreads];

        for (int i = 0; i < kNumThreads; i++) {
            switch (params.randomType()) {
            case RandomType::MT19937:
                samplers[i] = Random::factory((unsigned int)(time(0) + i));
                break;

            case RandomType::Halton:
                samplers[i] = Halton::factory(250, true, 
                                              (unsigned int)(time(0) + i));
                break;

            default:
                std::cerr << "Unknown random sampler type !!" << std::endl;
                std::abort();
            }
        }

        // Distribute tasks
        const int np = params.castPhotons();
        const int taskPerThread = (np + kNumThreads - 1) / kNumThreads;
        std::vector<std::vector<Photon> > photons(kNumThreads);

        // Shooting photons
        int proc = 0;
        for (int t = 0; t < taskPerThread; t++) {
            ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {                
                // Request random numbers in each thread
                Stack<double> rstk;
                samplers[threadID].request(&rstk, 250);

                // Generate sample on the light
                Photon photon = scene.samplePhoton(rstk);
                
                Spectrum flux = photon.flux() / params.castPhotons();

                Vector3D dir;
                sampler::onHemisphere(photon.normal(), &dir,
                                      rstk.pop(), rstk.pop());

                // Trace photon
                Ray ray(photon.position(), dir);
                tracePhoton(scene, ray, params, flux, 
                            rstk, 0, absorbBsdf, &photons[threadID]);
            }

            proc += kNumThreads;
            if (proc % 1000 == 0) {
                printf("%6.2f %% processed...\r", 
                        100.0 * proc / params.castPhotons());
            }
        }
        printf("\n");

        int numStored = 0;
        for (int i = 0; i < kNumThreads; i++) {
            numStored += photons[i].size();
        }
        printf("[INFO] %d photons stored.\n", numStored);

        // Deallocate memories
        delete[] samplers;

        // Contruct tree structure
        printf("Constructing photon map -> ");
        
        clear();
        std::vector<Photon> photonsAll;
        for (int i = 0; i < kNumThreads; i++) {
            photonsAll.insert(photonsAll.end(), 
                              photons[i].begin(), photons[i].end());
        }
        _kdtree.construct(photonsAll);
        printf("OK\n");
    }

    Spectrum PhotonMap::evaluate(const Point& position,
                                 const Normal& normal,
                                 int gatherPhotons, double gatherRadius) const {
        // Find k-nearest neightbors
        Photon query(position, Spectrum(), Vector3D(), normal);
        std::vector<Photon> photons;
        knnFind(query, &photons, gatherPhotons, gatherRadius);

        const int numPhotons = static_cast<int>(photons.size());

        std::vector<Photon> validPhotons;
        std::vector<double> distances;
        double maxdist = 0.0;
        for (int i = 0; i < numPhotons; i++) {
            const Vector3D diff = query.position() - photons[i].position();
            const double dist = diff.norm();
            const double dt   = vect::dot(normal, diff) / dist;
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
            const Spectrum v = Spectrum(photons[i].flux() * INV_PI);
            totalFlux += w * v;
        }
        totalFlux /= (1.0 - 2.0 / (3.0 * k));

        if (maxdist > EPS) {
            return Spectrum(totalFlux / (PI * maxdist * maxdist));
        }
        return Spectrum(0.0, 0.0, 0.0);

    }

    void PhotonMap::knnFind(const Photon& photon, std::vector<Photon>* photons,
                            const int gatherPhotons,
                            const double gatherRadius) const {
        KnnQuery query(K_NEAREST | EPSILON_BALL, gatherPhotons, gatherRadius);
        _kdtree.knnSearch(photon, query, photons);
    }

    void PhotonMap::tracePhoton(const Scene& scene,
                                const Ray& ray,
                                const RenderParameters& params,
                                const Spectrum& flux,
                                Stack<double>& rstk,
                                int bounces,
                                BsdfType absorbBsdf,
                                std::vector<Photon>* photons) {
        // Too many bounces terminate recursion
        if (bounces >= params.bounceLimit()) {
            return;
        }

        // Remove photon with zero flux
        if (max3(flux.red(), flux.green(), flux.blue()) <= 0.0) {
            return;
        }

        // If not hit the scene, then break
        Intersection isect;
        if (!scene.intersect(ray, &isect)) {
            return;
        }

        // Hitting object
        const int       objID  = isect.objectID();
        const BSDF&     bsdf   = scene.getBsdf(objID);
        const Spectrum&    refl   = isect.color();

        const bool into = vect::dot(isect.normal(), ray.direction()) < 0.0;
        const Normal orientNormal = (into ? 1.0 : -1.0) * isect.normal();

        if (scene.isLightCheck(objID)) {
            return;
        }

        double photonPdf = 1.0;
        if (bsdf.type() & absorbBsdf) {
            photons->push_back(Photon(isect.position(), flux, 
                                      ray.direction(), isect.normal()));
            // Russian roulette
            const double prob = (refl.red() + refl.green() + refl.blue()) / 3.0;
            if (rstk.pop() < prob) {
                // Reflection
                photonPdf *= prob;
            } else {
                // Absorption
                return;
            }
        }

        double samplePdf = 1.0;
        Vector3D nextdir;
        bsdf.sample(ray.direction(), isect.normal(), rstk.pop(), rstk.pop(),
                    &nextdir, &samplePdf);

        Ray nextRay(isect.position(), nextdir);
        Spectrum nextFlux = Spectrum((flux * refl) / (samplePdf * photonPdf));

        // Next bounce
        tracePhoton(scene, nextRay, params, nextFlux, 
                    rstk, bounces + 1, absorbBsdf, photons);
    }

}  // namespace spica
