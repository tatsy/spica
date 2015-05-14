#define SPICA_MLT_RENDERER_EXPORT
#include "mlt_renderer.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>

#include "renderer_helper.h"
#include "../utils/sampler.h"

namespace spica {

    namespace {

        struct PrimarySample {
            int modify_time;
            double value;
            static const Random rnd;

            PrimarySample() {
                modify_time = 0;
                value = rnd.randReal();
            }
        };

        const Random PrimarySample::rnd = Random::getRNG();

        struct KelemenMLT {
        private:
            static const int num_init_primary_samples = 128;

            inline double mutate(const double x) {
                const double r = rng->randReal();
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
            const Random* rng;

            std::vector<PrimarySample> primary_samples;
            std::stack<PrimarySample> primary_samples_stack;

            KelemenMLT(const Random& rng = Random::getRNG()) 
                : global_time(0)
                , large_step(0)
                , large_step_time(0)
                , used_rand_coords(0)
                , rng(&rng)
                , primary_samples()
                , primary_samples_stack() 
            {
                primary_samples.resize(num_init_primary_samples);
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
                        primary_samples[used_rand_coords].value = rng->randReal();
                    } else {
                        if (primary_samples[used_rand_coords].modify_time < large_step_time) {
                            primary_samples[used_rand_coords].modify_time = large_step_time;
                            primary_samples[used_rand_coords].value = rng->randReal();
                        }

                        while (primary_samples[used_rand_coords].modify_time < global_time - 1) {
                            primary_samples[used_rand_coords].value = mutate(primary_samples[used_rand_coords].value);
                            primary_samples[used_rand_coords].modify_time++;
                        }

                        primary_samples_stack.push(primary_samples[used_rand_coords]);
                        primary_samples[used_rand_coords].value = mutate(primary_samples[used_rand_coords].value);
                        primary_samples[used_rand_coords].modify_time = global_time;
                    }
                }
                used_rand_coords++;
                return primary_samples[used_rand_coords - 1].value;
            }
        };

        double luminance(const Color& color) {
            return Vector3(0.2126, 0.7152, 0.0722).dot(color);
        }

        Color direct_radiance_sample(const Scene& scene, const Vector3& v0, const Vector3& normal, const int id, KelemenMLT& mlt) {
            const double r1 = 2.0 * PI * mlt.nextSample();
            const double r2 = 1.0 - 2.0 * mlt.nextSample();

            const Primitive* light = scene.get(scene.lightID());
            const Material& lightMtrl = scene.getMaterial(scene.lightID());
            Vector3 light_pos, light_normal;
            sampler::on(light, &light_pos, &light_normal);

            const Vector3 v_to_l = light_pos - v0;
            const Vector3 light_dir = v_to_l.normalized();
            const double dist2 = v_to_l.dot(v_to_l);
            const double dot0 = normal.dot(light_dir);
            const double dot1 = light_normal.dot(-1.0 * light_dir);

            if (dot0 >= 0.0 && dot1 >= 0.0) {
                const double G = dot0 * dot1 / dist2;
                Intersection intersection;
                if (scene.intersect(Ray(v0, light_dir), intersection) && intersection.objectId() == scene.lightID()) {
                    const Material& mtrl = scene.getMaterial(id);
                    return mtrl.color.cwiseMultiply(lightMtrl.emission) * (1.0 / PI) * G * light->area();
                }
            }
            return Color(0.0, 0.0, 0.0);
        }

        Color radiance(const Scene& scene, const Ray& ray, const int depth, const int maxDepth, KelemenMLT& mlt) {
            Intersection intersection;
            if (!scene.intersect(ray, intersection)) {
                return Color(0.0, 0.0, 0.0);
            }

            const Material& mtrl = scene.getMaterial(intersection.objectId());
            const Hitpoint& hitpoint = intersection.hitpoint();
            const Vector3 orient_normal = hitpoint.normal().dot(ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            const Color& obj_color = mtrl.color;
            double roulette_probability = std::max(obj_color.red(), std::max(obj_color.green(), obj_color.blue()));

            if (depth > maxDepth) {
                if (mlt.nextSample() >= roulette_probability) {
                    return Color(0.0, 0.0, 0.0);
                }
            } else {
                roulette_probability = 1.0;
            }

            if (mtrl.reftype == REFLECTION_DIFFUSE) {
                if (intersection.objectId() != scene.lightID()) {
                    const int shadow_ray = 1;
                    Vector3 direct_light;
                    for (int i = 0; i < shadow_ray; i++) {
                        direct_light = direct_light + direct_radiance_sample(scene, hitpoint.position(), orient_normal, intersection.objectId(), mlt) / shadow_ray;
                    }

                    Vector3 nextDir;
                    sampler::onHemisphere(orient_normal, &nextDir);
                    const Color next_bounce_color = radiance(scene, Ray(hitpoint.position(), nextDir), depth + 1, maxDepth, mlt);
                    return (direct_light + obj_color.cwiseMultiply(next_bounce_color)) / roulette_probability;
                } else if (depth == 0) {
                    return mtrl.emission;
                } else {
                    return Color(0.0, 0.0, 0.0);
                }
            }
            else if (mtrl.reftype == REFLECTION_SPECULAR) {
                Intersection light_intersect;
                Ray reflection_ray = Ray(hitpoint.position(), Vector3::reflect(ray.direction(), hitpoint.normal()));
                scene.intersect(reflection_ray, light_intersect);
                Vector3 direct_light;
                if (light_intersect.objectId() == scene.lightID()) {
                    direct_light = scene.getMaterial(scene.lightID()).emission;
                }
                const Color next_bounce_color = radiance(scene, reflection_ray, depth + 1, maxDepth, mlt);
                return (direct_light + obj_color.cwiseMultiply(next_bounce_color)) / roulette_probability;
            } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                Intersection light_intersect;
                Ray reflection_ray = Ray(hitpoint.position(), Vector3::reflect(ray.direction(), hitpoint.normal()));
                scene.intersect(reflection_ray, light_intersect);
                Vector3 direct_light;
                if (light_intersect.objectId() == scene.lightID()) {
                    direct_light = scene.getMaterial(scene.lightID()).emission;
                }

                bool isIncoming = hitpoint.normal().dot(orient_normal) > 0.0;

                Vector3 reflectDir, refractDir;
                double fresnelRe, fresnelTr;
                if (helper::isTotalRef(isIncoming, hitpoint.position(), ray.direction(), hitpoint.normal(), orient_normal, &reflectDir, &refractDir, &fresnelRe, &fresnelTr)) {
                    // Total reflection
                    const Color next_bounce_color = radiance(scene, reflection_ray, depth + 1, maxDepth, mlt);
                    return (direct_light + obj_color.cwiseMultiply(next_bounce_color)) / roulette_probability;
                }

                Ray refraction_ray = Ray(hitpoint.position(), refractDir);
                Intersection light_intersect_refract;
                scene.intersect(reflection_ray, light_intersect_refract);
                Vector3 direct_light_refraction;
                if (light_intersect_refract.objectId() == scene.lightID()) {
                    direct_light_refraction = scene.getMaterial(scene.lightID()).emission;
                }

                const double probability = 0.25 + 0.5 * fresnelRe;

                if (depth > 2) {
                    if (mlt.nextSample() < probability) {
                        const Color next_bounce_color = radiance(scene, reflection_ray, depth + 1, maxDepth, mlt);
                        return obj_color.cwiseMultiply(fresnelRe * (direct_light + next_bounce_color)) / (probability * roulette_probability);
                    } else {
                        const Color next_bounce_color = radiance(scene, refraction_ray, depth + 1, maxDepth, mlt);
                        return obj_color.cwiseMultiply(fresnelTr * (direct_light_refraction + next_bounce_color)) / ((1.0 - probability) * roulette_probability);
                    }
                } else {
                    const Color next_bounce_color_reflect = radiance(scene, reflection_ray, depth + 1, maxDepth, mlt);
                    const Color next_bounce_color_refract = radiance(scene, refraction_ray, depth + 1, maxDepth, mlt);
                    const Color next_bounce_color = fresnelRe * (direct_light + next_bounce_color_reflect) 
                                                  + fresnelTr * (direct_light_refraction + next_bounce_color_refract);
                    return obj_color.cwiseMultiply(next_bounce_color) / roulette_probability;
                }
            }
            return Color(0.0, 0.0, 0.0);
        }

        struct PathSample {
            int x, y;
            Color F;
            double weight;
            PathSample(const int x_ = 0, const int y_ = 0, const Color& F_ = Color(), const double weight_ = 1.0)
                : x(x_)
                , y(y_)
                , F(F_)
                , weight(weight_)
            {
            }
        };

        PathSample generateNewPath(const Scene& scene, const Camera& camera, KelemenMLT& mlt, int x, int y, int maxDepth) {
            const int width  = camera.imageW();
            const int height = camera.imageH();
            
            double weight = 1.0;

            // Consider sampling probability on image
            if (x < 0) {
                weight *= width;
                x = mlt.nextSample() * width;
                if (x == width) {
                    x = 0;
                }
            }

            if (y < 0) {
                weight *= height;
                y = mlt.nextSample() * height;
                if (y == height) {
                    y = 0;
                }
            }

            // Position on object plane
            const double uOnSensor = static_cast<double>(x) / width - 0.5;
            const double vOnSensor = static_cast<double>(y) / height - 0.5;
            Vector3 posOnSensor = camera.center() + (uOnSensor * camera.sensorW()) * camera.sensorU() + (vOnSensor * camera.sensorH()) * camera.sensorV();

            const double ratio = camera.focalLength() / camera.distSL();
            const double uOnObjplane = -ratio * uOnSensor;
            const double vOnObjplane = -ratio * vOnSensor;
            Vector3 posOnOP = camera.objplaneCenter()
                            + (uOnObjplane * camera.objplaneW()) * camera.objplaneU()
                            + (vOnObjplane * camera.objplaneH()) * camera.objplaneV();

            // Sample point on lens
            double r0 = sqrt(mlt.nextSample());
            double r1 = 2.0 * PI * mlt.nextSample();
            double rx = r0 * cos(r1);
            double ry = r0 * sin(r1);
            Vector3 posOnLens = camera.lensCenter() 
                              + camera.lensRadius() * camera.lensU() * rx 
                              + camera.lensRadius() * camera.lensV() * ry;

            const double pImage = 1.0 / (camera.cellW() * camera.cellH());
            const double pLens  = 1.0 / (camera.lensArea());

            Vector3 lens2sensor = posOnSensor - posOnLens;
            const double cosine = Vector3::dot(camera.direction(), lens2sensor.normalized());
            const double coeff  = cosine * cosine / lens2sensor.squaredNorm();
            weight *= (coeff * camera.sensitivity() / (pImage * pLens));


            const Ray ray = Ray(posOnLens, (posOnOP - posOnLens).normalized());
            Color c = radiance(scene, ray, 0, maxDepth, mlt);

            return PathSample(x, y, c, weight);
        }

    }  // anonymous namespace

    MLTRenderer::MLTRenderer()
    {
    }

    MLTRenderer::~MLTRenderer()
    {
    }

    int MLTRenderer::render(const Scene& scene, const Camera& camera, const Random& rng, int numMLT, int numMutate, int maxDepth) {
        const int width  = camera.imageW();
        const int height = camera.imageH();
        Image image(width, height);

        for (int mi = 0; mi < numMLT; mi++) {
            Image tmpImage(width, height);
            KelemenMLT kelemenMlt(rng);

            int seed_path_max = width * height;
            if (seed_path_max <= 0) {
                seed_path_max = 1;
            }

            std::vector<PathSample> seed_paths(seed_path_max);
            double sumI = 0.0;
            kelemenMlt.large_step = 1;
            for (int i = 0; i < seed_path_max; i++) {
                kelemenMlt.initUsedRandCoords();
                PathSample sample = generateNewPath(scene, camera, kelemenMlt, -1, -1, maxDepth);
                kelemenMlt.global_time++;
                while (!kelemenMlt.primary_samples_stack.empty()) {
                    kelemenMlt.primary_samples_stack.pop();
                }

                sumI += luminance(sample.F);
                seed_paths[i] = sample;
            }

            // Compute first path
            const double rnd = rng.randReal() * sumI;
            int selected_path = 0;
            double accumulated_importance = 0.0;
            for (int i = 0; i < seed_path_max; i++) {
                accumulated_importance += luminance(seed_paths[i].F);
                if (accumulated_importance >= rnd) {
                    selected_path = i;
                    break;
                }
            }

            // --
            const double b = sumI / seed_path_max;
            const double p_large = 0.5;
            int accept = 0;
            int reject = 0;
            PathSample old_path = seed_paths[selected_path];
            int progress = 0;
            for (int i = 0; i < numMutate; i++) {
                if ((i + 1) % (numMutate / 10) == 0) {
                    progress += 10;
                    std::cout << progress << " % ";
                    std::cout << "Accept: " << accept << ", Reject: " << reject;
                    std::cout << ", Rate: " << (100.0 * accept / (accept + reject)) << " %" << std::endl;
                }

                kelemenMlt.large_step = rng.randReal() < p_large ? 1 : 0;
                kelemenMlt.initUsedRandCoords();
                PathSample new_path = generateNewPath(scene, camera, kelemenMlt, -1, -1, maxDepth);

                double a = std::min(1.0, luminance(new_path.F) / luminance(old_path.F));
                const double new_path_weight = (a + kelemenMlt.large_step) / (luminance(new_path.F) / b + p_large) / numMutate;
                const double old_path_weight = (1.0 - a) / (luminance(old_path.F) / b + p_large) / numMutate;

                tmpImage.pixel(new_path.x, new_path.y) += new_path.weight * new_path_weight * new_path.F;
                tmpImage.pixel(old_path.x, old_path.y) += old_path.weight * old_path_weight * old_path.F;
                
                if (rng.randReal() < a) {  // Accept
                    accept++;
                    old_path = new_path;
                    if (kelemenMlt.large_step) {
                        kelemenMlt.large_step_time = kelemenMlt.global_time;
                    }
                    kelemenMlt.global_time++;
                    while (!kelemenMlt.primary_samples_stack.empty()) {
                        kelemenMlt.primary_samples_stack.pop();
                    }
                } else {  // Reject
                    reject++;
                    int idx = kelemenMlt.used_rand_coords - 1;
                    while (!kelemenMlt.primary_samples_stack.empty()) {
                        kelemenMlt.primary_samples[idx--] = kelemenMlt.primary_samples_stack.top();
                        kelemenMlt.primary_samples_stack.pop();
                    }
                }
            }

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    image.pixel(width - x - 1, y) += tmpImage.pixel(x, y) / numMLT;
                }
            }
        }

        image.savePPM("simplemlt.ppm");
        return 0;
    }

}  // namespace spica
