#define SPICA_MLT_RENDERER_EXPORT
#include "mlt_renderer.h"

#include <algorithm>
#include <vector>
#include <stack>

namespace spica {

    namespace {

        struct PrimarySample {
            int modify_time;
            double value;
            PrimarySample() {
                modify_time = 0;
                value = rng.randReal();
            }
        };

        struct KelemenMLT {
        private:
            inline double mutate(const double x) {
                const double r = rng.randReal();
                const double s1 = 1.0 / 512.0;
                const double s2 = 1.0 / 16.0;
                const double dx = s1 / (s1 / s2 + abs(2.0 * r - 1.0)) - s1 / (s1 / s2 + 1.0);
                if (r < 0.5) {
                    const double x1 = x + dx;
                    return (x1 < 1.0) ? x1 : x1 - 1.0;
                } else {
                    const double x1 = x - dx;
                    return (x1 < 0.0) ? x1 + 1.0 : x1;
                }
            }

        public:
            int global_time;
            int large_step;
            int large_step_time;
            int used_rand_coords;

            std::vector<PrimarySample> primary_samples;
            std::stack<PrimarySample> primary_samples_stack;

            KelemenMLT() {
                global_time = large_step = large_step_time = used_rand_coords = 0;
                primary_samples.resize(128);
            }

            void initUsedRandCoords() {
                used_rand_coords = 0;
            }

            inline double nextSample() {
                if (primary_samples.size() <= used_rand_coords) {
                    primary_samples.resize(primary_samples.size() * 1.5);
                }

                if (primary_samples[used_rand_coords].modify_time < global_time) {
                    if (large_step > 0) {
                        primary_samples_stack.push(primary_samples[used_rand_coords]);
                        primary_samples[used_rand_coords].modify_time = global_time;
                        primary_samples[used_rand_coords].value = rng.randReal();
                    } else {
                        if (primary_samples[used_rand_coords].modify_time < large_step_time) {
                            primary_samples[used_rand_coords].modify_time = large_step_time;
                            primary_samples[used_rand_coords].value = rng.randReal();
                        }

                        while (primary_samples[used_rand_coords].modify_time < global_time - 1) {
                            primary_samples[used_rand_coords].value = mutate(primary_samples[used_rand_coords].value);
                            primary_samples[used_rand_coords].modify_time++;
                        }

                        primary_samples_stack.push(primary_samples[used_rand_coords]);
                        primary_samples[used_rand_coords].value = mutate(primary_samples[used_rand_coords].value);
                        primary_samples[used_rand_coords].modify_time = global_time;
                    }

                    used_rand_coords++;
                    return primary_samples[used_rand_coords - 1].value;
                }
            }
        };

        Color radiance(const Scene& scene, const Ray& ray, const int depth, KelemenMLT& mlt) {
            Intersection intersection;
            if (!scene.intersect(ray, intersection)) {
                return Color(0.0, 0.0, 0.0);
            }

            const Primitive* obj_ptr = scene.getObjectPtr(intersection.objectId());
            const HitPoint& hitpoint = intersection.hitPoint();
            const Vector3 orient_normal = hitpoint.normal().dot(ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            const Color& light_color = obj_ptr->color();
            double roulette_probability = std::max(light_color.red(), std::max(light_color.green(), light_color.blue()));

            if (depth > maxDepth) {
                if (mlt.nextSample() >= roulette_probability) {
                    return Color(0.0, 0.0, 0.0);
                }
            } else {
                roulette_probability = 1.0;
            }

            if (obj_ptr->reftype() == REFLECTION_DIFFUSE) {
                if (intersection.objectId() != scene.lightId()) {
                    const int shadow_ray = 1;
                    Vector3 direct_light;
                    for (int i = 0; i < shadow_ray; i++) {
                        direct_light = direct_light + direct_radiance_sample(hitpoint.position(), orient_normal, intersection.objectId(), mlt) / shadow_ray;
                    }

                    Vector3 w, u, v;
                    w = orient_normal;
                    if (abs(w.x()) > EPS) {
                        u = Vector3(0.0, 1.0, 0.0).cross(w).normalize();
                    } else {
                        u = Vector3(1.0, 0.0, 0.0).cross(w).normalize();
                    }
                    v = w.cross(u);

                    const double r1 = 2.0 * PI * mlt.nextSample();
                    const double r2 = mlt.nextSample();
                    const double r2s = sqrt(r2);
                    Vector3 next_dir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0 - r2)).normalize();

                    const Color next_bounce_color = radiance(scene, Ray(hitpoint.position(), next_dir), depth + 1, mlt);
                    return direct_light + light_color.cwiseMultiply(next_bounce_color) / roulette_probability;
                } else if (depth == 0) {
                    return obj_ptr->emission();
                } else {
                    return Color(0.0, 0.0, 0.0);
                }
            }
            else if (obj_ptr->reftype() == REFLECTION_SPECULAR) {
                Intersection light_intersect;
                Ray reflection_ray = Ray(hitpoint.position(), ray.direction() - hitpoint.normal() * 2.0 * hitpoint.normal().dot(ray.direction()));
                scene.intersect(reflection_ray, light_intersect);
                Vector3 direct_light;
                if (light_intersect.objectId() == scene.lightId()) {
                    direct_light = scene.getObjectPtr(scene.lightId())->emission();
                }
                const Color next_bounce_color = radiance(scene, reflection_ray, depth + 1, mlt);
                return direct_light + obj_ptr->color().cwiseMultiply(next_bounce_color) / roulette_probability;
            } else if (obj_ptr->reftype() == REFLECTION_REFRACTION) {
                Intersection light_intersect;
                Ray reflection_ray = Ray(hitpoint.position(), ray.direction() - hitpoint.normal() * 2.0 * hitpoint.normal().dot(ray.direction()));
                scene.intersect(reflection_ray, light_intersect);
                Vector3 direct_light;
                if (light_intersect.objectId() == scene.lightId()) {
                    direct_light = scene.getObjectPtr(scene.lightId())->emission();
                }

                bool is_incoming = hitpoint.normal().dot(orient_normal) > 0.0;

                // Snell
                const double nc = 1.0;
                const double nt = 1.5;
                const double nnt = is_incoming ? nc / nt : nt / nc;
                const double ddn = ray.direction().dot(orient_normal);
                const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

                if (cos2t < 0.0) {
                    const Color next_bounce_color = radiance(scene, reflection_ray, depth + 1, mlt);
                    return direct_light + obj_ptr->color().cwiseMultiply(next_bounce_color) / roulette_probability;
                }

                Vector3 tdir = (ray.direction() * nnt - hitpoint.normal() * (is_incoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t)));

                // Schlick
                const double a = nt - nc;
                const double b = nt + nc;
                const double R0 = (a * a) / (b * b);
                const double c = 1.0 - (is_incoming ? -ddn : tdir.dot(hitpoint.normal()));
                const double Re = R0 + (1.0 - R0) * pow(c, 5.0);
                const double Tr = 1.0 - Re;
                const double probability = 0.25 + 0.5 * Re;

                Ray refraction_ray = Ray(hitpoint.position(), tdir);
                scene.intersect(reflection_ray, light_intersect);
                Vector3 direct_light_refraction;
                if (light_intersect.objectId() == scene.lightId()) {
                    direct_light_refraction = scene.getObjectPtr(scene.lightId())->emission();
                }

                if (depth > 2) {
                    if (mlt.nextSample() < probability) {
                        const Color next_bounce_color = radiance(scene, reflection_ray, depth + 1, mlt);
                        return obj_ptr->color().cwiseMultiply(direct_light + Re * next_bounce_color) / (probability * roulette_probability);
                    } else {
                        const Color next_bounce_color = radiance(scene, refraction_ray, depth + 1, mlt);
                        return obj_ptr->color().cwiseMultiply(direct_light_refraction + Tr * next_bounce_color) / ((1.0 - probability) * roulette_probability);
                    }
                } else {
                    const Color next_bounce_color_reflect = radiance(scene, reflection_ray, depth + 1, mlt);
                    const Color next_bounce_color_refract = radiance(scene, refraction_ray, depth + 1, mlt);
                    const Color next_bounce_color = Re * next_bounce_color_reflect + Tr * next_bounce_color_refract;
                    return obj_ptr->color().cwiseMultiply(direct_light + next_bounce_color) / roulette_probability;
                }
            }
            return Color(0.0, 0.0, 0.0);
        }
    }

    MLTRenderer::MLTRenderer()
    {
    }

    MLTRenderer::~MLTRenderer()
    {
    }

    int MLTRenderer::render(const Scene& scene, const Camera& camera) {
    }

}  // namespace spica
