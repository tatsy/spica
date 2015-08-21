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
                *refractDir = Vector3D(0.0, 0.0, 0.0);
                *fresnelRef = 1.0;
                *fresnelTransmit = 0.0;
                return true;
            }

            *refractDir = (in * nnt - normal * (isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalized();

            const double a = kIorObject - kIorVaccum;
            const double b = kIorObject + kIorVaccum;
            const double R0 = (a * a) / (b * b);

            const double c = 1.0 - (isIncoming ? -ddn : Vector3D::dot(*refractDir, -orientNormal));
            *fresnelRef = R0 + (1.0 - R0) * (c * c * c * c * c);
            *fresnelTransmit = 1.0 - (*fresnelRef);

            return false;
        }

        Color radiance(const Scene& scene, const RenderParameters& params,
                       const Ray& ray, Stack<double>& rands, const int bounces) {
            if (bounces >= params.bounceLimit()) {
                return Color::BLACK;
            }

            Intersection isect;
            if (!scene.intersect(ray, isect)) {
                return scene.envmap().sampleFromDir(ray.direction());
            }

            // Require random numbers
            const double randnums[3] = { rands.pop(), rands.pop(), rands.pop() };

            // Get intersecting material
            const int objectID     = isect.objectId();
            const BSDF& bsdf       = scene.getBsdf(objectID);
            const Color& refl      = bsdf.reflectance();
            const Color& emittance = scene.getEmittance(objectID);
            const Hitpoint& hpoint = isect.hitpoint();

            // Russian roulette
            double roulette = max3(refl.red(), refl.green(), refl.blue());
            if (bounces < params.bounceStartRoulette()) {
                roulette = 1.0;
            } else {
                if (roulette <= randnums[0]) {
                    return emittance;
                }
            }

            // Sample next direction
            double pdf = 1.0;
            Vector3D nextdir;
            bsdf.sample(ray.direction(), hpoint.normal(), 
                        randnums[1], randnums[2], &nextdir, &pdf);
            
            Ray nextray(hpoint.position(), nextdir);
            const Color nextrad = radiance(scene, params, nextray,
                                           rands, bounces + 1);
            
            // Return result
            return Color(emittance + refl * nextrad / (roulette * pdf));       
        }

        Color directLight(const Scene& scene,
                          const Vector3D& pos,
                          const Vector3D& in,
                          const Vector3D& normal,
                          const BSDF& bsdf,
                          Stack<double>& rstk) {

            const bool      into = Vector3D::dot(normal, in) < 0.0;
            const Vector3D  orientNormal = into ? normal : -normal;

            if (bsdf.type() & BSDF_TYPE_LAMBERTIAN_BRDF) {
                const int       lightID  = scene.sampleLight(rstk.pop());
                const Triangle& light    = scene.getTriangle(lightID);
                const Color&    lightEmt = scene.getEmittance(lightID);

                const double r1Light = rstk.pop();
                const double r2Light = rstk.pop();

                Vector3D light_pos, light_normal;
                sampler::onTriangle(light, &light_pos, &light_normal, r1Light, r2Light);

                const Vector3D v_to_l = light_pos - pos;
                const Vector3D light_dir = v_to_l.normalized();
                const double dist2 = v_to_l.squaredNorm();
                const double dot0 = orientNormal.dot(light_dir);
                const double dot1 = light_normal.dot(-1.0 * light_dir);

                if (dot0 >= 0.0 && dot1 >= 0.0) {
                    const double G = dot0 * dot1 / dist2;
                    Intersection isect;
                    if (scene.intersect(Ray(pos, light_dir), isect) && isect.objectId() == lightID) {
                        return Color(lightEmt * (INV_PI * G * light.area()));
                    }
                }
            } else {
                double pdf = 1.0;
                Vector3D nextdir;
                bsdf.sample(in, normal, rstk.pop(), rstk.pop(), &nextdir, &pdf);
                const Ray refRay(pos, nextdir);

                Intersection isect;
                if (scene.intersect(refRay, isect) && scene.isLightCheck(isect.objectId())) {
                    return Color(scene.getEmittance(isect.objectId()) / pdf);
                }
            }
            return Color(0.0, 0.0, 0.0);
        }

    }

}