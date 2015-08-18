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

        bool checkTotalReflection(const bool isIncoming,
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

        Color radiance(const Scene& scene, const Ray& ray, Stack<double>& rands, const int depth, const int depthLimit, const int depthMin) {
            if (depth >= depthLimit) {
                return Color::BLACK;
            }

            Intersection isect;
            if (!scene.intersect(ray, isect)) {
                return scene.envmap().sampleFromDir(ray.direction());
            }

            // Require random numbers
            const double randnums[3] = { rands.pop(), rands.pop(), rands.pop() };

            // Get intersecting material
            const int objectID = isect.objectId();
            const BSDF& bsdf = scene.getBsdf(objectID);
            const Color& emittance = scene.getEmittance(objectID);
            const Hitpoint& hitpoint = isect.hitpoint();

            // Russian roulette
            double roulette = std::max(bsdf.reflectance().red(), std::max(bsdf.reflectance().green(), bsdf.reflectance().blue()));
            if (depth < depthMin) {
                roulette = 1.0;
            } else {
                if (roulette <= randnums[0]) {
                    return emittance;
                }
            }

            // Sample next direction
            double pdf = 1.0;
            Vector3D nextdir;
            bsdf.sample(ray.direction(), hitpoint.normal(), randnums[1], randnums[2], &nextdir, &pdf);
            
            Ray nextray(hitpoint.position(), nextdir);
            const Color nextrad = radiance(scene, nextray, rands, depth + 1, depthLimit, depthMin);
            
            // Return result
            return Color(emittance + bsdf.reflectance() * nextrad / (roulette * pdf));       
        }

    }

}