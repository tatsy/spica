#define SPICA_RENDERER_HELPER_EXPORT
#include "renderer_helper.h"

#include <cmath>
#include <algorithm>

#include "../utils/sampler.h"

namespace spica {

    namespace helper {
        bool isTotalRef(const bool isIncoming,
            const Vector3& position,
            const Vector3& in,
            const Vector3& normal,
            const Vector3& orientNormal,
            Vector3* reflectDir,
            Vector3* refractDir,
            double* fresnelRef,
            double* fresnelTransmit) {

            *reflectDir = Vector3::reflect(in, normal);

            // Snell's rule
            const double nnt = isIncoming ? IOR_VACCUM / IOR_OBJECT : IOR_OBJECT / IOR_VACCUM;
            const double ddn = in.dot(orientNormal);
            const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

            if (cos2t < 0.0) {
                // Total reflect
                *refractDir = Vector3();
                *fresnelRef = 1.0;
                *fresnelTransmit = 0.0;
                return true;
            }

            *refractDir = (in * nnt - normal * (isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalized();

            const double a = IOR_OBJECT - IOR_VACCUM;
            const double b = IOR_OBJECT + IOR_VACCUM;
            const double R0 = (a * a) / (b * b);

            const double c = 1.0 - (isIncoming ? -ddn : Vector3::dot(*refractDir, -orientNormal));
            *fresnelRef = R0 + (1.0 - R0) * pow(c, 5.0);
            *fresnelTransmit = 1.0 - (*fresnelRef);

            return false;
        }

        Color radiance(const Scene& scene, const Ray& ray, const Random& rng, const int depth, const int depthLimit, const int maxDepth) {
            Intersection isect;

            // NOT intersect the scene
            if (!scene.intersect(ray, isect)) {
                return scene.bgColor();
            }

            const Material& mtrl = scene.getMaterial(isect.objectId());
            const Hitpoint& hitpoint = isect.hitpoint();
            const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            double roulette = std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));

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
            } else if (mtrl.reftype == REFLECTION_SUBSURFACE) {
                Vector3 nextDir;
                sampler::onHemisphere(orientNormal, &nextDir);
                incomingRad = radiance(scene, Ray(hitpoint.position(), nextDir), rng, depth + 1);
                weight = mtrl.color / roulette;
            }

            return mtrl.emission + weight.cwiseMultiply(incomingRad);
        }

    }

}