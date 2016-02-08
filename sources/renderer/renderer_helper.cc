#define SPICA_API_EXPORT
#include "renderer_helper.h"

#include <cmath>
#include <algorithm>

#include "../core/spectrum.h"
#include "../core/sampler.h"

#include "../math/vector3d.h"

#include "../renderer/ray.h"
#include "../light/lighting.h"

#include "../bsdf/bsdf.h"
#include "../scenes/scene.h"
#include "../camera/camera.h"

#include "render_parameters.h"
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

        bool checkTotalReflection(const bool into,
                                  const Vector3D& in,
                                  const Normal& n,
                                  const Vector3D& on,
                                  Vector3D* reflectDir,
                                  Vector3D* transmitDir,
                                  double* fresnelRe,
                                  double* fresnelTr) {

            *reflectDir = vect::reflect(in, n).normalized();

            // Snell's rule
            const double nnt = into ? kIorVaccum / kIorObject 
                                    : kIorObject / kIorVaccum;
            const double ddn = in.dot(on);
            const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

            if (cos2t < 0.0) {
                // Total reflect
                *transmitDir = { 0.0, 0.0, 0.0 };
                *fresnelRe   = 1.0;
                *fresnelTr   = 0.0;
                return true;
            }

            *transmitDir = (in * nnt - n * (into ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalized();

            const double a = kIorObject - kIorVaccum;
            const double b = kIorObject + kIorVaccum;
            const double R0 = (a * a) / (b * b);

            const double c = 1.0 - (into ? -ddn : vect::dot(*transmitDir, n));
            *fresnelRe = R0 + (1.0 - R0) * (c * c * c * c * c);
            *fresnelTr = (1.0 - (*fresnelRe)) * (nnt * nnt);

            return false;
        }

        Spectrum radiance(const Scene& scene, const RenderParameters& params,
                       const Ray& ray, Stack<double>& rands, int bounces) {
            if (bounces >= params.bounceLimit()) {
                return Spectrum(0.0, 0.0, 0.0);
            }

            Intersection isect;
            if (!scene.intersect(ray, &isect)) {
                return Spectrum(0.0, 0.0, 0.0);
            }

            // Require random numbers
            const double randnums[3] = { rands.pop(), rands.pop(), rands.pop() };

            // Get intersecting material
            const int objectID     = isect.objectID();
            const BSDF& bsdf       = scene.getBsdf(objectID);
            const Spectrum& refl      = isect.color();

            // Russian roulette
            double roulette = max3(refl.red(), refl.green(), refl.blue());
            if (bounces < params.bounceStartRoulette()) {
                roulette = 1.0;
            } else {
                if (roulette <= randnums[0]) {
                    return Spectrum(0.0, 0.0, 0.0);
                }
            }

            // Sample next direction
            double pdf = 1.0;
            Vector3D nextdir;
            bsdf.sample(ray.direction(), isect.normal(), 
                        randnums[1], randnums[2], &nextdir, &pdf);
            
            Ray nextray(isect.position(), nextdir);
            const Spectrum nextrad = radiance(scene, params, nextray,
                                           rands, bounces + 1);
            
            // Return result
            return Spectrum(refl * nextrad / (roulette * pdf));       
        }

        Spectrum directLight(const Scene& scene,
                          const Point& pos,
                          const Vector3D& in,
                          const Normal& normal,
                          const BSDF& bsdf,
                          Stack<double>& rstk) {
            const bool   into = vect::dot(normal, in) < 0.0;
            const Normal orientNormal = into ? normal : -normal;

            if (bsdf.type() & BsdfType::Lambertian) {
                LightSample Ls = scene.sampleLight(pos, rstk);
                if (Ls.pdf() != 0.0) {
                    Intersection isect;
                    if (scene.intersect(Ray(pos, -Ls.dir()), &isect)) {
                        if ((isect.position() - Ls.position()).norm() < EPS) {
                            double dot = vect::dot(normal, -Ls.dir());
                            return Ls.Le() * std::abs(dot) * Ls.pdf();
                        }
                    }
                }
            } else {
                double pdf = 1.0;
                Vector3D nextdir;
                bsdf.sample(in, normal, rstk.pop(), rstk.pop(), &nextdir, &pdf);
                const Ray refRay(pos, nextdir);

                Intersection isect;
                if (scene.intersect(refRay, &isect) && scene.isLightCheck(isect.objectID())) {
                    return scene.directLight(nextdir) / pdf;
                }
            }
            return Spectrum(0.0, 0.0, 0.0);
        }

    }

}