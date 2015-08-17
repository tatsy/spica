#define SPICA_RENDERER_HELPER_EXPORT
#include "renderer_helper.h"

#include <cmath>
#include <algorithm>

#include "../utils/sampler.h"

#include "renderer_constants.h"

namespace spica {

    namespace helper {

        void calcLocalCoords(const Vector3D& w, Vector3D* u, Vector3D* v) {
            if (std::abs(w.x()) > 0.1) {
                *u = Vector3D(0.0, 1.0, 0.0).cross(w).normalized();
            } else {
                *u = Vector3D(1.0, 0.0, 0.0).cross(w).normalized();
            }
            *v = w.cross(*u);
        }

        bool isTotalRef(const bool isIncoming,
                        const Vector3D& position,
                        const Vector3D& in,
                        const Vector3D& normal,
                        const Vector3D& orientNormal,
                        Vector3D* reflectDir,
                        Vector3D* refractDir,
                        double* fresnelRef,
                        double* fresnelTransmit) {

            *reflectDir = Vector3D::reflect(in, normal);

            // Snell's rule
            const double nnt = isIncoming ? kIorVaccum / kIorObject : kIorObject / kIorVaccum;
            const double ddn = in.dot(orientNormal);
            const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

            if (cos2t < 0.0) {
                // Total reflect
                *refractDir = Vector3D();
                *fresnelRef = 1.0;
                *fresnelTransmit = 0.0;
                return true;
            }

            *refractDir = (in * nnt - normal * (isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalized();

            const double a = kIorObject - kIorVaccum;
            const double b = kIorObject + kIorVaccum;
            const double R0 = (a * a) / (b * b);

            const double c = 1.0 - (isIncoming ? -ddn : Vector3D::dot(*refractDir, -orientNormal));
            *fresnelRef = R0 + (1.0 - R0) * pow(c, 5.0);
            *fresnelTransmit = 1.0 - (*fresnelRef);

            return false;
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
            const BSDF& bsdf = scene.getBsdf(objectID);
            const Hitpoint& hitpoint = isect.hitpoint();
            const Vector3D orientNormal = Vector3D::dot(ray.direction(), hitpoint.normal()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            // If depth is over depthLimit, terminate recursion
            if (depth >= depthLimit) {
                return bsdf.emittance();
            }

            // Russian roulette
            double roulette = std::max(bsdf.reflectance().red(), std::max(bsdf.reflectance().green(), bsdf.reflectance().blue()));
            if (depth > depthMin) {
                if (roulette < randnums[0]) {
                    return bsdf.emittance();
                }
            } else {
                roulette = 1.0;
            }

            // Handle hitting materials
            Color weight(1.0, 1.0, 1.0);
            Color nextRad(1.0, 1.0, 1.0);

            // Sample next direction
            double pdf = 1.0;
            Vector3D nextdir;
            bsdf.sample(ray.direction(), hitpoint.normal(), randnums[1], randnums[2], &nextdir, &pdf);
            Color nextrad = radiance(scene, Ray(hitpoint.position(), nextdir), rseq, depth + 1, depthLimit, depthMin);
            
            // Return result
            return Color(bsdf.emittance() + weight.multiply(nextRad));        
        }

    }

}