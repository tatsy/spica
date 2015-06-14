#define SPICA_PHOTON_MAPPING_EXPORT
#include "photon_mapping.h"

#include <cmath>
#include <ctime>
#include <cstdio>
#include <algorithm>

#include "renderer_helper.h"
#include "../utils/sampler.h"

namespace spica {

    PMRenderer::PMRenderer()
    {
    }

    PMRenderer::~PMRenderer()
    {
    }

    void PMRenderer::render(const Scene& scene, const Camera& camera, const int samplePerPixel, const PMParams& params, const RandomType randType) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        Image* buffer = new Image[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {    
            buffer[i] = Image(width, height);
        }

        RandomBase** rand = new RandomBase*[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {
            switch (randType) {
            case PSEUDO_RANDOM_TWISTER:
                rand[i] = new Random();
                break;
            case QUASI_MONTE_CARLO:
                rand[i] = new Halton(200, true, i);
                break;
            }
        }

        const int taskPerThread = (samplePerPixel + OMP_NUM_CORE - 1) / OMP_NUM_CORE;
        for (int t = 0; t < taskPerThread; t++) {
            // Construct photon map
            buildPM(scene, camera, params.numPhotons, randType);

            // Path tracing
            ompfor (int threadID = 0; threadID < OMP_NUM_CORE; threadID++) {
                RandomSeq rseq;            
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        rand[threadID]->requestSamples(rseq, 200);
                        buffer[threadID].pixel(width - x - 1, y) += executePathTracing(scene, camera, rseq, x, y, params.gatherPhotons, params.gatherRadius);
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
            sprintf(filename, "photonmap_%03d.bmp", usedSamples);
            image.saveBMP(filename);

            printf("  %6.2f %%  processed -> %s\r", 100.0 * (t + 1) / taskPerThread, filename);
        }
        printf("\nFinish!!\n");

        for (int i = 0; i < OMP_NUM_CORE; i++) {
            delete rand[i];
        }
        delete[] rand;
        delete[] buffer;
    }

    Color PMRenderer::executePathTracing(const Scene& scene, const Camera& camera, RandomSeq& rseq, const double pixelX, const double pixelY, const int numTargetPhotons, const double targetRadius) const {
        CameraSample camSample = camera.sample(pixelX, pixelY, rseq);
        const Ray ray = camSample.generateRay();
        return radiance(scene, ray, rseq, numTargetPhotons, targetRadius, 0) * (camera.sensitivity() / camSample.totalPdf());
    }    
    
    Color PMRenderer::radiance(const Scene& scene, const Ray& ray, RandomSeq& rseq, const int numTargetPhotons, const double targetRadius, const int depth, const int depthLimit, const int maxDepth) const {
        Intersection isect;
        if (!scene.intersect(ray, isect)) {
            return scene.envmap(ray.direction());
        }

        // Request random numbers
        const double randnum = rseq.next();

        // Intersected object
        const int objID = isect.objectId();
        const Material& mtrl = scene.getMaterial(objID);
        const Hitpoint hitpoint = isect.hitpoint();
        const Vector3 orientNormal = Vector3::dot(ray.direction(), hitpoint.normal()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

        // Russian roulette
        double roulette = std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));
        if (depth > maxDepth) {
            if (randnum > roulette) {
                return mtrl.emission;
            }
        }
        roulette = 1.0;

        if (mtrl.reftype == REFLECTION_DIFFUSE) {
            // Estimate irradiance with photon map
            Photon query = Photon(hitpoint.position(), Color(), ray.direction(), hitpoint.normal());
            std::vector<Photon> photons;
            photonMap.findKNN(query, &photons, numTargetPhotons, targetRadius);

            const int numPhotons = static_cast<int>(photons.size());

            std::vector<Photon> validPhotons;
            std::vector<double> distances;
            double maxdist = 0.0;
            for (int i = 0; i < numPhotons; i++) {
                Vector3 diff = query - photons[i];
                double dist = diff.norm();
                if (std::abs(Vector3::dot(hitpoint.normal(), diff)) < diff.norm() * 0.1) {
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
                const Color v = mtrl.color.multiply(photons[i].flux()) / PI;
                totalFlux += w * v;
            }
            totalFlux /= (1.0 - 2.0 / (3.0 * k));
            
            if (maxdist > EPS) {
                return mtrl.emission + totalFlux / ((PI * maxdist * maxdist) * roulette); 
            }
        } else if (mtrl.reftype == REFLECTION_SPECULAR) {
            Vector3 nextDir = Vector3::reflect(ray.direction(), hitpoint.normal());
            Ray nextRay = Ray(hitpoint.position(), nextDir);
            Color nextRad = radiance(scene, nextRay, rseq, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
            return mtrl.emission + mtrl.color.multiply(nextRad) / roulette;
        } else if (mtrl.reftype == REFLECTION_REFRACTION) {
            bool isIncoming = Vector3::dot(hitpoint.normal(), orientNormal) > 0.0;
            Vector3 reflectDir, refractDir;
            double fresnelRe, fresnelTr;
            if (helper::isTotalRef(isIncoming, hitpoint.position(), ray.direction(), hitpoint.normal(), orientNormal, &reflectDir, &refractDir, &fresnelRe, &fresnelTr)) {
                // Total reflection
                Ray nextRay = Ray(hitpoint.position(), reflectDir);
                Color nextRad = radiance(scene, nextRay, rseq, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                return mtrl.emission + mtrl.color.multiply(nextRad) / roulette;
            } else {
                if (depth > 2) {
                    // Reflect or reflact
                    const double probRef = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                    if (randnum < probRef) {
                        // Reflect
                        Ray nextRay = Ray(hitpoint.position(), reflectDir);
                        Color nextRad = radiance(scene, nextRay, rseq, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                        return mtrl.emission + mtrl.color.multiply(nextRad) * (fresnelRe / (probRef * roulette));
                    } else {
                        // Refract
                        Ray nextRay = Ray(hitpoint.position(), refractDir);
                        Color nextRad = radiance(scene, nextRay, rseq, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                        return mtrl.emission + mtrl.color.multiply(nextRad) * (fresnelTr / ((1.0 - probRef) * roulette));
                    }
                } else {
                    Ray reflectRay = Ray(hitpoint.position(), reflectDir);
                    Color reflectRad = radiance(scene, reflectRay, rseq, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                    Ray transmitRay = Ray(hitpoint.position(), refractDir);
                    Color transmitRad = radiance(scene, transmitRay, rseq, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                    Color nextRad = reflectRad * fresnelRe + transmitRad * fresnelTr;
                    return mtrl.emission + mtrl.color.multiply(nextRad) / roulette;
                }
            }
        }

        return Color();
    }

    void PMRenderer::buildPM(const Scene& scene, const Camera& camera, const int numPhotons, const RandomType randType) {
        std::cout << "Shooting photons..." << std::endl;

        // Prepare random number generator
        RandomBase* rand = NULL;
        switch (randType) {
        case PSEUDO_RANDOM_TWISTER:
            rand = new Random();
            break;
        case QUASI_MONTE_CARLO:
            rand = new Halton();
            break;
        }

        // Shooting photons
        std::vector<Photon> photons;
        int proc = 0;
        ompfor (int pid = 0; pid < numPhotons; pid++) {
            
            // Request random numbers in each thread
            RandomSeq rseq;
            omplock {
                rand->requestSamples(rseq, 200);
            }

            // Generate sample on the light
            const int lightID = scene.lightID();
            const Primitive* light = scene.get(lightID);

            Vector3 posLignt, normalLight;
            const double r1Light = rseq.next();
            const double r2Light = rseq.next();
            sampler::on(light, &posLignt, &normalLight, r1Light, r2Light);
            Color currentFlux = light->area() * scene.getMaterial(lightID).emission * PI / numPhotons;

            const double r1 = rseq.next();
            const double r2 = rseq.next();
            Vector3 nextDir;
            sampler::onHemisphere(normalLight, &nextDir, r1, r2);

            Ray currentRay(posLignt, nextDir);

            for (;;) {
                const double randnum = rseq.next();

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
                const Material& mtrl = scene.getMaterial(isect.objectId());
                const Hitpoint& hitpoint = isect.hitpoint();

                Vector3 orientingNormal = hitpoint.normal();
                if (Vector3::dot(hitpoint.normal(), currentRay.direction()) > 0.0) {
                    orientingNormal *= -1.0;
                }

                if (mtrl.reftype == REFLECTION_DIFFUSE) {
                    omplock {
                        photons.push_back(Photon(hitpoint.position(), currentFlux, currentRay.direction(), hitpoint.normal()));
                    }

                    const double probContinueTrace = (mtrl.color.red() + mtrl.color.green() + mtrl.color.blue()) / 3.0;
                    if (probContinueTrace > randnum) {
                        // Continue trace
                        sampler::onHemisphere(orientingNormal, &nextDir);
                        currentRay = Ray(hitpoint.position(), nextDir);
                        currentFlux = currentFlux.multiply(mtrl.color) / probContinueTrace;
                    } else {
                        // Absorb (finish trace)
                        break;
                    }
                } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                    nextDir = Vector3::reflect(currentRay.direction(), hitpoint.normal());
                    currentRay = Ray(hitpoint.position(), nextDir);
                    currentFlux = currentFlux.multiply(mtrl.color);
                } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                    bool isIncoming = Vector3::dot(hitpoint.normal(), orientingNormal) > 0.0;

                    Vector3 reflectDir, transmitDir;
                    double fresnelRe, fresnelTr;
                    bool isTotRef = helper::isTotalRef(isIncoming,
                                                       hitpoint.position(),
                                                       currentRay.direction(),
                                                       hitpoint.normal(),
                                                       orientingNormal,
                                                       &reflectDir,
                                                       &transmitDir,
                                                       &fresnelRe,
                                                       &fresnelTr);


                    Ray reflectRay = Ray(hitpoint.position(), reflectDir);

                    if (isTotRef) {
                        // Total reflection
                        currentRay = reflectRay;
                        currentFlux = currentFlux.multiply(mtrl.color);
                        continue;
                    }

                    const double probability = 0.25 + REFLECT_PROBABLITY * fresnelRe;

                    if (randnum < probability) {
                        // Reflection
                        currentRay = reflectRay;
                        currentFlux = currentFlux.multiply(mtrl.color) * (fresnelRe / probability);
                    } else {
                        // Reflaction
                        currentRay = Ray(hitpoint.position(), transmitDir);
                        currentFlux = currentFlux.multiply(mtrl.color) * (fresnelTr / (1.0 - probability));
                    }
                } else {
                    msg_assert(false, "Unknown reflection type !!");
                }
            }

            omplock {
                proc++;
                if (proc % 1000 == 0) {
                    printf("%6.2f %% processed...\r", 100.0 * proc / numPhotons);
                }
            }
        }
        printf("\n\n");

        // Release random number generator
        delete rand;

        // Construct photon map
        printf("Constructing photon map -> ");
        photonMap.clear();
        photonMap.construct(photons);
        printf("OK\n");
    }

}

