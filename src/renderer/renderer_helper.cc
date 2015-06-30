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

        Color radiance(const Scene& scene, const Ray& ray, Random& rng, const int depth, const int depthLimit, const int maxDepth) {
            Intersection isect;
            if (!scene.intersect(ray, isect)) {
                return scene.envmap().sampleFromDir(ray.direction());
            }

            const Material& mtrl = scene.getMaterial(isect.objectId());
            const Hitpoint& hitpoint = isect.hitpoint();
            const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            double roulette = std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));

            if (depth > depthLimit) {
                return mtrl.emission;
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
                Vector3 nextDir = Vector3::reflect(ray.direction(), orientNormal);
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
                    if (rng.nextReal() < prob) {
                        // Reflect
                        incomingRad = radiance(scene, reflectRay, rng, depth + 1) * fresnelRe;
                        weight = mtrl.color / (prob * roulette);
                    } else {
                        // Transmit
                        incomingRad = radiance(scene, transmitRay, rng, depth + 1) * fresnelTr;
                        weight = mtrl.color / ((1.0 - prob) * roulette);
                    }
                }
            } else if (mtrl.reftype == REFLECTION_SUBSURFACE) {
                Vector3 nextDir;
                sampler::onHemisphere(orientNormal, &nextDir);
                incomingRad = radiance(scene, Ray(hitpoint.position(), nextDir), rng, depth + 1);
                weight = mtrl.color / roulette;
            }

            return Color(mtrl.emission + weight.multiply(incomingRad));
        }

        Color radiance(const Scene& scene, const Ray& ray, RandomSeq& rseq, const int depth, const int depthLimit, const int depthMin) {
            Intersection isect;
            if (!scene.intersect(ray, isect)) {
                return scene.envmap().sampleFromDir(ray.direction());
            }

            // Require random numbers
            std::vector<double> randnums;
            rseq.next(3, &randnums);

            // Get intersecting material
            const int objectID = isect.objectId();
            const Material& mtrl = scene.getMaterial(objectID);
            const Hitpoint& hitpoint = isect.hitpoint();
            const Vector3 orientNormal = Vector3::dot(ray.direction(), hitpoint.normal()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            // If depth is over depthLimit, terminate recursion
            if (depth >= depthLimit) {
                return mtrl.emission;
            }

            // Russian roulette
            double roulette = std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));
            if (depth > depthMin) {
                if (roulette < randnums[0]) {
                    return mtrl.emission;
                }
            } else {
                roulette = 1.0;
            }

            // Handle hitting materials
            Color weight(1.0, 1.0, 1.0);
            Color nextRad(1.0, 1.0, 1.0);

            // TODO: BRDF support here is temporal
            // in the future, reftype is removed and 
            // all the material types are converted to BRDF
            if (mtrl.reftype == REFLECTION_BRDF) {
                Vector3 nextDir;
                mtrl.brdf.sample(ray.direction(), orientNormal, randnums[1], randnums[2], &nextDir);
                Ray nextRay(hitpoint.position(), nextDir);
                weight = weight.multiply(mtrl.brdf.reflectance()) / roulette;
                nextRad = radiance(scene, nextRay, rseq, depth + 1, depthLimit, depthMin);
            } else if (mtrl.reftype == REFLECTION_DIFFUSE) {
                // Diffuse reflection
                // Sample next direction with QMC
                Vector3 u, v, w;
                w = orientNormal;
                if (std::abs(w.x()) > EPS) {
                    u = Vector3(0.0, 1.0, 0.0).cross(w).normalized();
                } else {
                    u = Vector3(1.0, 0.0, 0.0).cross(w).normalized();
                }
                v = w.cross(u);

                const double r1 = 2.0 * PI * randnums[1];
                const double r2 = randnums[2];
                const double r2s = sqrt(r2);

                Vector3 nextDir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0 - r2)).normalized();
                Ray nextRay(hitpoint.position(), nextDir);
                weight = weight.multiply(mtrl.color) / roulette;
                nextRad = radiance(scene, nextRay, rseq, depth + 1, depthLimit, depthMin);

            } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                // Specular reflection
                Vector3 nextDir = Vector3::reflect(ray.direction(), orientNormal);
                Ray nextRay(hitpoint.position(), nextDir);
                weight = weight.multiply(mtrl.color) / roulette;
                nextRad = radiance(scene, nextRay, rseq, depth + 1, depthLimit, depthMin);
            } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                // Refraction
                bool isInto = Vector3::dot(hitpoint.normal(), orientNormal) > 0.0;              
                Vector3 reflectDir, transmitDir;
                double fresnelRe, fresnelTr;
                bool isTotRef = helper::isTotalRef(isInto,
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
                    Ray nextRay(hitpoint.position(), reflectDir);
                    weight = weight.multiply(mtrl.color) / roulette;
                    nextRad = radiance(scene, nextRay, rseq, depth + 1, depthLimit, depthMin);
                } else {
                    // Trace either reflect or transmit ray
                    const double refProb = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                    if (randnums[1] < refProb) {
                        Ray nextRay(hitpoint.position(), reflectDir);
                        weight = weight.multiply(mtrl.color) / (refProb * roulette);
                        nextRad = radiance(scene, nextRay, rseq, depth + 1, depthLimit, depthMin) * fresnelRe;
                    } else {
                        Ray nextRay(hitpoint.position(), transmitDir);
                        weight = weight.multiply(mtrl.color) / ((1.0 - refProb) * roulette);
                        nextRad = radiance(scene, nextRay, rseq, depth + 1, depthLimit, depthMin) * fresnelTr;
                    }
                }
            }

            return Color(mtrl.emission + weight.multiply(nextRad));        
        }

    }

}