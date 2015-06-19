#define SPICA_PROGRESSIVE_PHOTONMAP_EXPORT
#include "progressive_photonmap.h"

#include <cstdio>
#include <iostream>
#include <algorithm>

#include "renderer_helper.h"
#include "../utils/sampler.h"

namespace spica {

    const double PPMRenderer::ALPHA = 0.7;

    PPMRenderer::PPMRenderer()
    {
    }

    PPMRenderer::~PPMRenderer()
    {
    }

    void PPMRenderer::render(const Scene& scene, const Camera& camera, const int samplePerPixel, const int numPhotons, const RandomType randType) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        const int numPoints = width * height;
        std::vector<HPoint> hpoints(numPoints);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int id = y * width + x;
                hpoints[id].imageX = x;
                hpoints[id].imageY = y;
                hpoints[id].n = 0;
            }
        }

        RandomBase* rand = NULL;
        switch (randType) {
        case PSEUDO_RANDOM_TWISTER:
            rand = new Random();
            break;
        case QUASI_MONTE_CARLO:
            rand = new Halton();
            break;
        default:
            msg_assert(false, "Unknown random number generator type!!");
            break;
        }

        for (int t = 0; t < samplePerPixel; t++) {
            std::cout << "--- Iteration No." << (t + 1) << " ---" << std::endl;
            // 1st pass: Trace rays from camera
            traceRays(scene, camera, rand, hpoints);

            // 2nd pass: Trace photons from light source
            tracePhotons(scene, rand, numPhotons);

            // Save temporal image
            Image image(width, height);
            for (int i = 0; i < numPoints; i++) {
                const HPoint& hp = hpoints[i];
                image.pixel(width - hp.imageX - 1, hp.imageY) += (hp.emission + hp.flux / (PI * hp.r2)) * (hp.coeff / (t + 1));
            }

            char filename[256];
            sprintf(filename, "ppm_%02d.bmp", t + 1);
            image.saveBMP(filename);
        }

        // Release memories
        delete rand;
    }

    void PPMRenderer::constructHashGrid(std::vector<HPoint>& hpoints, const int imageW, const int imageH) {

        // Clear current data
        hashgrid.clear();

        const int numPoints = static_cast<int>(hpoints.size());

        // Compute bounding box
        BBox bbox;
        for (int i = 0; i < numPoints; i++) {
            bbox.merge(static_cast<Vector3>(hpoints[i]));
        }

        // Heuristic for initial radius
        Vector3 boxSize = bbox.posMax() - bbox.posMin();
        const double irad = ((boxSize.x() + boxSize.y() + boxSize.z()) / 3.0) / ((imageW + imageH) / 2.0) * 2.0;

        // Update initial radius
        Vector3 iradv(irad, irad, irad);
        for (int i = 0; i < numPoints; i++) {
            if (hpoints[i].n == 0) {
                hpoints[i].r2 = irad * irad;
                hpoints[i].n = 0;
                hpoints[i].flux = Color(0.0, 0.0, 0.0);
            }

            bbox.merge(hpoints[i] + iradv);
            bbox.merge(hpoints[i] - iradv);
        }
        
        // Make each grid cell two times larger than the initial radius
        const double hashScale = 1.0 / (irad * 2.0);
        const int hashSize = numPoints;

        hashgrid.init(hashSize, hashScale, bbox);

        // Set hit points to the grid
        for (int i = 0; i < numPoints; i++) {
            Vector3 boxMin = static_cast<Vector3>(hpoints[i]) - iradv;
            Vector3 boxMax = static_cast<Vector3>(hpoints[i]) + iradv;
            hashgrid.add(&hpoints[i], boxMin, boxMax);
        }
    }

    void PPMRenderer::traceRays(const Scene& scene, const Camera& camera, RandomBase* rand, std::vector<HPoint>& hpoints) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        const int numPoints = static_cast<int>(hpoints.size());

        // Generate a ray to cast
        std::cout << "Tracing rays from camera ..." << std::endl;

        // Distribute Hitpoints to each thread
        std::vector<std::vector<int> > hpIDs(OMP_NUM_CORE);
        for (int i = 0; i < numPoints; i++) {
            hpIDs[i % OMP_NUM_CORE].push_back(i);
        }

        int proc = 0;
        ompfor (int threadID = 0; threadID < OMP_NUM_CORE; threadID++) {
            const int taskPerThread = static_cast<int>(hpIDs[threadID].size());
            for (int i = 0; i < taskPerThread; i++) {
                RandomSeq rseq;
                omplock {
                    rand->requestSamples(rseq, 200);
                }

                const int hpID = hpIDs[threadID][i];
                executePathTracing(scene, camera, rseq, &hpoints[hpID]);

                omplock {
                    proc += 1;
                    if (proc % width == 0) {
                        printf("%6.2f %% processed...\r", 100.0 * proc / numPoints);
                    }
                }
            }
        }
        printf("\nFinish !!\n");

        // Construct k-d tree
        constructHashGrid(hpoints, width, height);
        std::cout << "Hash grid constructed !!" << std::endl << std::endl;
    }

    void PPMRenderer::tracePhotons(const Scene& scene, RandomBase* rand, const int numPhotons) {
        std::cout << "Shooting photons ..." << std::endl;
        int proc = 0;
        ompfor(int pid = 0; pid < numPhotons; pid++) {
            RandomSeq rseq;
            omplock {
                rand->requestSamples(rseq, 200);
            }

            // Sample point on light
            const int lightID = scene.lightID();
            const Primitive* light = scene.get(lightID);

            const double r1Light = rseq.next();
            const double r2Light = rseq.next();
            Vector3 posOnLight, normalOnLight;
            sampler::on(light, &posOnLight, &normalOnLight, r1Light, r2Light);

            // Compute flux
            Color currentFlux = Color(scene.getMaterial(lightID).emission * (light->area() * PI / numPhotons));

            // Prepare ray
            Vector3 nextDir;
            sampler::onHemisphere(normalOnLight, &nextDir);
            Ray currentRay(posOnLight, nextDir);
            Vector3 prevNormal = normalOnLight;

            // Shooting photons
            for (;;) {
                const double randnum = rseq.next();

                // Remove photons with zero flux
                if (std::max(currentFlux.red(), std::max(currentFlux.green(), currentFlux.blue())) <= 0.0) {
                    break;
                }

                // Intersection check
                Intersection isect;
                bool isHit = scene.intersect(currentRay, isect);
                if (!isHit) {
                    break;
                }

                const int objectID = isect.objectId();
                const Material& mtrl = scene.getMaterial(objectID);
                const Hitpoint& hitpoint = isect.hitpoint();
                const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), currentRay.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

                if (mtrl.reftype == REFLECTION_DIFFUSE) {
                    // Photon reaches diffuse surface. Update hitpoints.

                    // Gather hit points
                    std::vector<HPoint*> results;
                    omplock{
                        results = hashgrid[hitpoint.position()];
                    }

                    // Update hit points
                    for (int i = 0; i < results.size(); i++) {
                        HPoint* hpp = results[i];
                        Vector3 v = (*hpp) - hitpoint.position();
                        if (Vector3::dot(hpp->normal, hitpoint.normal()) > EPS && (v.squaredNorm() <= hpp->r2)) {
                            double g = (hpp->n * ALPHA + ALPHA) / (hpp->n * ALPHA + 1.0);
                            omplock{
                                hpp->r2 *= g;
                                hpp->n += 1;
                                hpp->flux = (hpp->flux + hpp->weight.multiply(currentFlux) * (1.0 / PI)) * g;
                            }
                        }
                    }

                    // Determine continue or terminate trace with Russian roulette
                    const double probability = (mtrl.color.red() + mtrl.color.green() + mtrl.color.blue()) / 3.0;
                    if (randnum < probability) {
                        sampler::onHemisphere(orientNormal, &nextDir);
                        currentRay = Ray(hitpoint.position(), nextDir);
                        currentFlux = currentFlux.multiply(mtrl.color) / probability;
                    } else {
                        break;
                    }
                } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                    nextDir = Vector3::reflect(currentRay.direction(), hitpoint.normal());
                    currentRay = Ray(hitpoint.position(), nextDir);
                    currentFlux = currentFlux.multiply(mtrl.color);
                } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                    bool isIncoming = Vector3::dot(hitpoint.normal(), orientNormal) > 0.0;
                    Vector3 reflectDir, refractDir;
                    double fresnelRe, fresnelTr;
                    bool isTotRef = helper::isTotalRef(isIncoming,
                                                       hitpoint.position(),
                                                       currentRay.direction(),
                                                       hitpoint.normal(),
                                                       orientNormal,
                                                       &reflectDir,
                                                       &refractDir,
                                                       &fresnelRe,
                                                       &fresnelTr);
                    if (isTotRef) {
                        // Total reflection
                        currentRay = Ray(hitpoint.position(), reflectDir);
                        currentFlux = currentFlux.multiply(mtrl.color);
                    } else {
                        // Trace either of reflect and transmit rays
                        const double probability = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                        if (randnum < probability) {
                            // Reflect
                            currentRay = Ray(hitpoint.position(), reflectDir);
                            currentFlux = currentFlux.multiply(mtrl.color) * (fresnelRe / probability);
                        } else {
                            // Transmit
                            currentRay = Ray(hitpoint.position(), refractDir);
                            currentFlux = currentFlux.multiply(mtrl.color) * (fresnelTr / (1.0 - probability));
                        }
                    }
                }
            }

            omplock{
                proc += 1;
                if (proc % 100 == 0) {
                    printf("%6.2f %% processed ...\r", 100.0 * proc / numPhotons);
                }
            }
        }
        printf("\nFinish !!\n\n");
    }

    void PPMRenderer::executePathTracing(const Scene& scene, const Camera& camera, RandomSeq& rseq, HPoint* hp, const int bounceLimit) {
        msg_assert(hp->imageX >= 0 && hp->imageY >= 0 && hp->imageX < camera.imageW() && hp->imageY < camera.imageH(), "Pixel index out of range");

        CameraSample camSample = camera.sample(hp->imageX, hp->imageY, rseq);
        Ray ray = camSample.generateRay();
        const double coeff = camera.sensitivity() / camSample.totalPdf();

        Intersection isect;
        Color weight(1.0, 1.0, 1.0);
        for (int bounce = 0;; bounce++) {
            double randnum = rseq.next();

            if (!scene.intersect(ray, isect) || bounce > bounceLimit) {
                weight = weight.multiply(scene.envmap(ray.direction()));
                hp->weight = weight;
                hp->coeff = coeff;
                break;
            }

            int objectID = isect.objectId();
            const Hitpoint& hitpoint = isect.hitpoint();
            const Material& mtrl = scene.getMaterial(objectID);
            const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            if (mtrl.reftype == REFLECTION_DIFFUSE) {
                // Ray hits diffuse object, return current weight
                weight = weight.multiply(mtrl.color);
                hp->setPosition(hitpoint.position());
                hp->normal = hitpoint.normal();
                hp->weight = weight;
                hp->coeff = coeff;
                hp->emission += weight.multiply(mtrl.emission);
                break;
            } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                Vector3 nextDir = Vector3::reflect(ray.direction(), orientNormal);
                ray = Ray(hitpoint.position(), nextDir);
                weight = weight.multiply(mtrl.color);
            } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                bool isIncoming = Vector3::dot(hitpoint.normal(), orientNormal) > 0.0;
                Vector3 reflectDir, transmitDir;
                double fresnelRe, fresnelTr;
                bool isTotRef = helper::isTotalRef(isIncoming,
                                                   hitpoint.position(),
                                                   ray.direction(),
                                                   hitpoint.normal(),
                                                   orientNormal,
                                                   &reflectDir,
                                                   &transmitDir,
                                                   &fresnelRe,
                                                   &fresnelTr);

                if (isTotRef) {
                    // Total reflection
                    ray = Ray(hitpoint.position(), reflectDir);
                    weight = weight.multiply(mtrl.color);
                } else {
                    // Trace either reflection or refraction ray with probability
                    const double probability = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                    if (randnum < probability) {
                        // Reflection
                        ray = Ray(hitpoint.position(), reflectDir);
                        weight = weight.multiply(mtrl.color) * (fresnelRe / probability);
                    } else {
                        // Transmit
                        ray = Ray(hitpoint.position(), transmitDir); 
                        weight = weight.multiply(mtrl.color) * (fresnelTr / (1.0 - probability));
                    }
                }
            }
        }
    }

}
