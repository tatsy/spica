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

    PMRenderer::PMRenderer()
    {
    }

    PMRenderer::~PMRenderer()
    {
    }

    void PMRenderer::render(const Scene& scene, const Camera& camera, const RenderParameters& params) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        Image* buffer = new Image[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {    
            buffer[i] = Image(width, height);
        }

        RandomBase** rand = new RandomBase*[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {
            switch (params.randomType()) {
            case PSEUDO_RANDOM_TWISTER:
                rand[i] = new Random();
                break;
            case QUASI_MONTE_CARLO:
                rand[i] = new Halton(250, true, i);
                break;
            }
        }

        const int taskPerThread = (params.samplePerPixel() + OMP_NUM_CORE - 1) / OMP_NUM_CORE;
        for (int t = 0; t < taskPerThread; t++) {
            // Construct photon map
            buildPM(scene, camera, params.castPhotons(), params.bounceLimit(), params.randomType());

            // Path tracing
            ompfor (int threadID = 0; threadID < OMP_NUM_CORE; threadID++) {
                Stack<double> rstk;            
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        rand[threadID]->request(&rstk, 250);
                        buffer[threadID].pixel(width - x - 1, y) += executePathTracing(scene, camera, params, rstk, x, y);
                    }
                }
            }

            const int usedSamples = (t + 1) * OMP_NUM_CORE;
            char filename[256];
            Image image(width, height);            
            for (int k = 0; k < OMP_NUM_CORE; k++) {
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        image.pixel(x, y) += buffer[k](x, y) / usedSamples;
                    }
                }
            }
            sprintf(filename, params.saveFilenameFormat().c_str(), usedSamples);
            image.gammaCorrect(1.0 / 2.2);
            image.save(filename);

            printf("  %6.2f %%  processed -> %s\r", 100.0 * (t + 1) / taskPerThread, filename);
        }
        printf("\nFinish!!\n");

        for (int i = 0; i < OMP_NUM_CORE; i++) {
            delete rand[i];
        }
        delete[] rand;
        delete[] buffer;
    }

    Color PMRenderer::executePathTracing(const Scene& scene, const Camera& camera, const RenderParameters& params, Stack<double>& rseq, const double pixelX, const double pixelY) const {
        CameraSample camSample = camera.sample(pixelX, pixelY, rseq);
        const Ray ray = camSample.generateRay();
        return Color(radiance(scene, params, ray, rseq, 0) * (camera.sensitivity() / camSample.totalPdf()));
    }    
    
    Color PMRenderer::radiance(const Scene& scene, const RenderParameters& params, const Ray& ray, Stack<double>& rseq, int bounces) const {
        if (bounces >= params.bounceLimit()) {
            return Color::BLACK;
        }

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
        const Hitpoint hitpoint = isect.hitpoint();
        const Vector3D orientNormal = Vector3D::dot(ray.direction(), hitpoint.normal()) < 0.0
                                    ? hitpoint.normal() : -hitpoint.normal();

        // Russian roulette
        double roulette = std::max(bsdf.reflectance().red(), std::max(bsdf.reflectance().green(), bsdf.reflectance().blue()));
        if (bounces > params.bounceStartRoulette()) {
            if (rands[0] > roulette) {
                return emission;
            }
        }
        roulette = 1.0;

        Color nextRad(0.0, 0.0, 0.0);
        if (bsdf.type() & BSDF_TYPE_LAMBERTIAN_BRDF) {
            // Estimate irradiance with photon map
            Photon query = Photon(hitpoint.position(), Color(), ray.direction(), hitpoint.normal());
            std::vector<Photon> photons;
            photonMap.findKNN(query, &photons, params.gatherPhotons(), params.gatherRadius());

            const int numPhotons = static_cast<int>(photons.size());

            std::vector<Photon> validPhotons;
            std::vector<double> distances;
            double maxdist = 0.0;
            for (int i = 0; i < numPhotons; i++) {
                const Vector3D diff = query.position() - photons[i].position();
                const double dist = diff.norm();
                if (std::abs(Vector3D::dot(hitpoint.normal(), diff) / dist) < params.gatherRadius() * params.gatherRadius() * 0.01) {
                    validPhotons.push_back(photons[i]);
                    distances.push_back(dist);
                    maxdist = std::max(maxdist, dist);
                }
            }

            // Cone filter
            const int numValidPhotons = static_cast<int>(validPhotons.size());
            const double k = 1.1;
            Color totalFlux = Color(0.0, 0.0, 0.0);
            for (int i = 0; i < numValidPhotons; i++) {
                const double w = 1.0 - (distances[i] / (k * maxdist));
                const Color v = Color(photons[i].flux() * INV_PI);
                totalFlux += w * v;
            }
            totalFlux /= (1.0 - 2.0 / (3.0 * k));
            
            if (maxdist > EPS) {
                nextRad = totalFlux / (PI * maxdist * maxdist);
            } 
        } else {
            double pdf = 1.0;
            Vector3D nextDir;
            bsdf.sample(ray.direction(), hitpoint.normal(), rands[1], rands[2], &nextDir, &pdf);
            Ray nextRay = Ray(hitpoint.position(), nextDir);
            nextRad = radiance(scene, params, nextRay, rseq, bounces + 1) / pdf;
        }

        return Color(emission + (bsdf.reflectance() * nextRad) / roulette);
    }

    void PMRenderer::buildPM(const Scene& scene, const Camera& camera, const int numPhotons, const int maxBounces, RandomType randType) {
        std::cout << "Shooting photons..." << std::endl;

        // Prepare random number generator
        RandomSampler sampler; 
        switch (randType) {
        case PSEUDO_RANDOM_TWISTER:
            sampler = Random::factory((unsigned int)time(NULL));
            break;

        case QUASI_MONTE_CARLO:
            sampler = Halton::factory(250, true, (unsigned int)time(NULL));
            break;

        default:
            std::cerr << "Unknown random sampler type !!" << std::endl;
            std::abort();
        }

        // Shooting photons
        std::vector<Photon> photons;
        int proc = 0;
        ompfor (int pid = 0; pid < numPhotons; pid++) {
            
            // Request random numbers in each thread
            Stack<double> rstk;
            omplock {
                sampler.request(&rstk, 250);
            }

            // Generate sample on the light
            const int       lightID  = scene.sampleLight(rstk.pop());
            const Triangle& light    = scene.getTriangle(lightID);
            const Color     lightEmt = scene.getEmittance(lightID);

            Vector3D posLight, normalLight;
            sampler::onTriangle(light, &posLight, &normalLight, rstk.pop(), rstk.pop());
            Color currentFlux = Color(light.area() * lightEmt * PI / numPhotons);

            Vector3D initdir;
            sampler::onHemisphere(normalLight, &initdir, rstk.pop(), rstk.pop());

            Ray currentRay(posLight, initdir);

            for (int bounces = 0; bounces < maxBounces; bounces++) {
                const double rands[3] = { rstk.pop(), rstk.pop(), rstk.pop() };

                // Remove photon with zero flux
                if (std::max(currentFlux.red(), std::max(currentFlux.green(), currentFlux.blue())) <= 0.0) {
                    break;
                }


                // If not hit the scene, then break
                Intersection isect;
                if (!scene.intersect(currentRay, isect)) {
                    break;
                }

                // Hitting object
                const BSDF& bsdf = scene.getBsdf(isect.objectId());
                const Hitpoint& hitpoint = isect.hitpoint();

                const Vector3D orientingNormal = Vector3D::dot(hitpoint.normal(), currentRay.direction()) < 0.0
                                               ? hitpoint.normal() : -hitpoint.normal();

                double photonPdf = 1.0;
                if (bsdf.type() & BSDF_TYPE_LAMBERTIAN_BRDF) {
                    omplock {
                        photons.push_back(Photon(hitpoint.position(), currentFlux, currentRay.direction(), hitpoint.normal()));
                    }

                    const double continueProbability = (bsdf.reflectance().red() + bsdf.reflectance().green() + bsdf.reflectance().blue()) / 3.0;
                    if (rands[0] < continueProbability) {
                        photonPdf *= continueProbability;
                    } else {
                        break;
                    }
                } 
                
                double samplePdf = 1.0;
                Vector3D nextdir;
                bsdf.sample(currentRay.direction(), hitpoint.normal(), rands[1], rands[2], &nextdir, &samplePdf);

                currentRay  = Ray(hitpoint.position(), nextdir);
                currentFlux = currentFlux * bsdf.reflectance() / (samplePdf * photonPdf);
            }

            omplock {
                proc++;
                if (proc % 1000 == 0) {
                    printf("%6.2f %% processed...\r", 100.0 * proc / numPhotons);
                }
            }
        }
        printf("\n");

        // Construct photon map
        printf("Constructing photon map -> ");
        photonMap.clear();
        photonMap.construct(photons);
        printf("OK\n");
    }

}  // namespace spica

