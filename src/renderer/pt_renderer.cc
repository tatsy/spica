#define SPICA_PT_RENDERER_EXPORT
#include "pt_renderer.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "../utils/common.h"
#include "../utils/vector3.h"
#include "../utils/sampler.h"
#include "../utils/image.h"
#include "material.h"
#include "scene.h"
#include "renderer_helper.h"

namespace spica {

    PTRenderer::PTRenderer()
    {
    }

    PTRenderer::PTRenderer(const PTRenderer& renderer)
    {
    }

    PTRenderer::~PTRenderer() 
    {
    }

    PTRenderer& PTRenderer::operator=(const PTRenderer& renderer) {
        return *this;
    }

    int PTRenderer::render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        // Vectors spanning screen
        Image image(width, height);
        int processed = 0;
        
        ompfor (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                for (int i = 0; i < samplePerPixel; i++) {
                    image.pixel(width - x - 1, y) += executePT(scene, camera, x, y, rng) / samplePerPixel;
                }
            }

            omplock {
                processed++;
                printf("%6.2f %% processed...\n", 100.0 * processed / height);
            }
        }
        
        image.saveBMP("path_trace.bmp");

        return 0;
    }

    Color PTRenderer::executePT(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, const Random& rng) {
        Vector3 posOnSensor;        // Position on the image sensor
        Vector3 posOnObjplane;      // Position on the object plane
        Vector3 posOnLens;          // Position on the lens
        double  pImage, pLens;      // Sampling probability on image sensor and lens

        camera.samplePoints(pixelX, pixelY, rng, posOnSensor, posOnObjplane, posOnLens, pImage, pLens);
        const Ray ray(posOnLens, Vector3::normalize(posOnObjplane - posOnLens));

        Vector3 lens2sensor = posOnSensor - posOnLens;
        const double cosine = Vector3::dot(camera.direction(), lens2sensor.normalized());
        const double weight = cosine * cosine / lens2sensor.squaredNorm();

        return radiance(scene, ray, rng, 0) * (weight * camera.sensitivity() / (pImage * pLens));
    }

    Color PTRenderer::radiance(const Scene& scene, const Ray& ray, const Random& rng, const int depth, const int depthLimit, const int maxDepth) {
        Intersection isect;

        // NOT intersect the scene
        if (!scene.intersect(ray, isect)) {
            return scene.bgColor();
        }

        const Material& mtrl = scene.getMaterial(isect.objectId());
        const Hitpoint& hitpoint = isect.hitpoint();
        const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

        double roulette =std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));

        if (depth > depthLimit) {
            roulette *= pow(0.5, depth - depthLimit);
        }

        if (depth > maxDepth) {
            if (roulette < rng.nextReal()) {
                return mtrl.emission;
            }
        } else {
            roulette = 1.0;
        }

        Color incomingRad;
        Color weight = Color(1.0, 1.0, 1.0);

        if (mtrl.reftype == REFLECTION_DIFFUSE) {
            Vector3 nextDir;
            sampler::onHemisphere(orientNormal, &nextDir);
            incomingRad = radiance(scene, Ray(hitpoint.position(), nextDir), rng, depth + 1);
            weight = mtrl.color / roulette;
        } else if (mtrl.reftype == REFLECTION_SPECULAR) {
            Vector3 nextDir = ray.direction() - (2.0 * hitpoint.normal().dot(ray.direction())) * hitpoint.normal();
            incomingRad = radiance(scene, Ray(hitpoint.position(), nextDir), rng, depth + 1);
            weight = mtrl.color / roulette;
        } else if (mtrl.reftype == REFLECTION_REFRACTION) {
            const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

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

            Ray reflectRay(hitpoint.position(), reflectDir);

            if (isTotRef) {
                // Total reflection
                incomingRad = radiance(scene, reflectRay, rng, depth + 1);
                weight = mtrl.color / roulette;
            } else {
                Ray transmitRay(hitpoint.position(), transmitDir);

                const double prob = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                if (depth > 2) {
                    if (rng.nextReal() < prob) {
                        // Reflect
                        incomingRad = radiance(scene, reflectRay, rng, depth + 1) * fresnelRe;
                        weight = mtrl.color / (prob * roulette);
                    } else {
                        // Transmit
                        incomingRad = radiance(scene, transmitRay, rng, depth + 1) * fresnelTr;
                        weight = mtrl.color / ((1.0 - prob) * roulette);
                    }
                } else {
                    // Both reflect and transmit
                    incomingRad = radiance(scene, reflectRay, rng, depth + 1) * fresnelRe + radiance(scene, transmitRay, rng, depth + 1) * fresnelTr;
                    weight = mtrl.color / roulette;
                }
            }
        }

        return mtrl.emission + weight.cwiseMultiply(incomingRad);
    }

}  // namespace spica
