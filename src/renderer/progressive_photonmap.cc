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

    PPMRenderer::PPMRenderer(const PPMRenderer& renderer)
    {
    }

    PPMRenderer::~PPMRenderer()
    {
    }

    int PPMRenderer::render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel, const int numPhotons) {
        // 1st pass: Trace rays from camera
        traceRays(scene, camera, rng, samplePerPixel);

        // 2nd pass: Trace photons from light source
        tracePhotons(scene, rng, numPhotons);

        const int width = camera.imageW();
        const int height = camera.imageH();
        Image image(width, height);
        
        const std::vector<HPoint>& points = hashgrid.hps;
        const int np = static_cast<int>(points.size());
        for (int i = 0; i < np; i++) {
            const HPoint& hp = points[i];
            image.pixel(width - hp.imageX - 1, hp.imageY) += hp.flux * (1.0 / (PI * hp.r2)) * hp.coeff / samplePerPixel;
        }
        image.saveBMP("ppm.bmp");
        return 0;
    }

    void PPMRenderer::traceRays(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        
        // Generate a ray to cast
        std::cout << "Tracing rays from camera ..." << std::endl;
        std::vector<HPoint> hpoints;

        int proc = 0;
        ompfor (int i = 0; i < samplePerPixel; i++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    HPoint hp = executePathTracing(scene, camera, rng, x, y);
                    omplock {
                        hpoints.push_back(hp);
                    }
                }

                omplock {
                    proc += 1;
                    printf("%6.2f %% processed...\r", 100.0 * proc / (samplePerPixel * height));
                }
            }
        }
        printf("\nFinish !!\n");

        // Construct k-d tree
        hashgrid.construct(hpoints, width, height);
        std::cout << "Hash grid constructed !!" << std::endl;
    }

    void PPMRenderer::tracePhotons(const Scene& scene, const Random& rng, const int numPhotons) {
        std::cout << "Shooting photons ..." << std::endl;
        int proc = 0;
        ompfor (int pid = 0; pid < numPhotons; pid++) {
            // Sample point on light
            const int lightID = scene.lightID();
            const Primitive* light = scene.get(lightID);

            Vector3 posOnLight, normalOnLight;
            sampler::on(light, &posOnLight, &normalOnLight);
            
            // Compute flux
            Color currentFlux = light->area() * scene.getMaterial(lightID).emission / numPhotons;

            // Prepare ray
            Vector3 nextDir;
            sampler::onHemisphere(normalOnLight, &nextDir);
            Ray currentRay(posOnLight, nextDir);
            Vector3 prevNormal = normalOnLight;

            // Shooting photons
            for (;;) {
                // Remove photons with zero flux
                if (std::max(currentFlux.red(), std::max(currentFlux.green(), currentFlux.blue())) < 0.0) {
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
                    const std::vector<HPoint*> results = hashgrid(hitpoint.position());

                    // Update hit points
                    for (int i = 0; i < results.size(); i++) {
                        HPoint* hpp = results[i];
                        Vector3 v = (*hpp) - hitpoint.position();
                        if (Vector3::dot(hpp->normal, hitpoint.normal()) > EPS && (v.squaredNorm() <= hpp->r2)) {
                            double g = (hpp->n * ALPHA + ALPHA) / (hpp->n * ALPHA + 1.0);
                            omplock {
                                hpp->r2   *= g;
                                hpp->n    += 1;
                                hpp->flux += hpp->weight.cwiseMultiply(currentFlux) * (1.0 / PI) * g;
                            }
                        }
                    }

                    // Determine continue or terminate trace with Russian roulette
                    const double probability = (mtrl.color.red() + mtrl.color.green() + mtrl.color.blue()) / 3.0;
                    if (probability > rng.nextReal()) {
                        sampler::onHemisphere(orientNormal, &nextDir);
                        currentRay = Ray(hitpoint.position(), nextDir);
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color) / probability;
                    } else {
                        break;
                    }
                } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                    nextDir = Vector3::reflect(currentRay.direction(), hitpoint.normal());
                    currentRay = Ray(hitpoint.position(), nextDir);
                    currentFlux = currentFlux.cwiseMultiply(mtrl.color);
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
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color);
                    } else {
                        // Trace either of reflect and transmit rays
                        const double probability = 0.25 + 0.5 * fresnelRe;
                        if (rng.nextReal() < probability) {
                            // Reflect
                            currentRay = Ray(hitpoint.position(), reflectDir);
                            currentFlux = currentFlux.cwiseMultiply(mtrl.color) * fresnelRe / probability;
                        } else {
                            // Transmit
                            currentRay = Ray(hitpoint.position(), refractDir);
                            currentFlux = currentFlux.cwiseMultiply(mtrl.color) * fresnelTr / (1.0 - probability);
                        }
                    }                
                }
            }

            omplock {
                proc += 1;
                if (proc % 100 == 0) {
                    printf("%6.2f %% processed ...\r", 100.0 * proc / numPhotons);
                }
            }
        }
        printf("\nFinish !!\n");
    }

    PPMRenderer::HPoint PPMRenderer::executePathTracing(const Scene& scene, const Camera& camera, const Random& rng, const int imageX, const int imageY, const int bounceLimit) {
        Vector3 posOnSensor, posOnObjplane, posOnLens;
        double pImage, pLens;
        Intersection isect;

        // Generate a path to trace
        camera.samplePoints(imageX, imageY, rng, posOnSensor, posOnObjplane, posOnLens, pImage, pLens);
        Ray ray(posOnLens, (posOnObjplane - posOnLens).normalized());

        const Vector3 lens2sensor = posOnSensor - posOnLens;
        const double cosine = Vector3::dot(camera.direction(), lens2sensor.normalized());
        const double coeff = (cosine * cosine * camera.sensitivity()) / (pImage * pLens * lens2sensor.squaredNorm());

        Color weight = Color(1.0, 1.0, 1.0);
        for (int bounce = 0; ; bounce++) {
            if (!scene.intersect(ray, isect)) {
                weight = weight.cwiseMultiply(scene.bgColor());
                return HPoint(Vector3(), Vector3(), Vector3(), 0.0, imageX, imageY);
            }

            int objectID = isect.objectId();
            const Hitpoint& hitpoint = isect.hitpoint();
            const Material& mtrl = scene.getMaterial(objectID);
            const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            if (mtrl.reftype == REFLECTION_DIFFUSE) {
                // Ray hits diffuse object, then store point
                weight = mtrl.emission + weight.cwiseMultiply(mtrl.color);
                return HPoint(hitpoint.position(),
                              hitpoint.normal(),
                              weight,
                              coeff,
                              imageX,
                              imageY);
            } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                Vector3 nextDir = Vector3::reflect(ray.direction(), orientNormal);
                ray = Ray(hitpoint.position(), nextDir);
                weight = mtrl.emission + weight.cwiseMultiply(mtrl.color);
            } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                bool isIncoming = Vector3::dot(hitpoint.normal(), orientNormal) > 0.0;
                Vector3 reflectDir, refractDir;
                double fresnelRe, fresnelTr;
                bool isTotRef = helper::isTotalRef(isIncoming,
                                                   hitpoint.position(),
                                                   ray.direction(),
                                                   hitpoint.normal(),
                                                   orientNormal,
                                                   &reflectDir, 
                                                   &refractDir, 
                                                   &fresnelRe,
                                                   &fresnelTr);
                
                if (isTotRef) {
                    // Total reflection
                    ray = Ray(hitpoint.position(), reflectDir);
                    weight = mtrl.emission + weight.cwiseMultiply(mtrl.color);
                } else {
                    // Trace either reflection or refraction ray with probability
                    const double probability = 0.25 + 0.5 * fresnelRe;
                    if (rng.nextReal() < fresnelRe) {
                        // Reflection
                        ray = Ray(hitpoint.position(), reflectDir);
                        weight = mtrl.emission + weight.cwiseMultiply(mtrl.color) * fresnelRe / probability;
                    } else {
                        // Transmit
                        ray = Ray(hitpoint.position(), refractDir);
                        weight = mtrl.emission + weight.cwiseMultiply(mtrl.color) * fresnelTr / (1.0 - probability);
                    }
                }
            }
        }
        return HPoint();
    }

}
