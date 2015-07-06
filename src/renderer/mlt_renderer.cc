#define SPICA_MLT_RENDERER_EXPORT
#include "mlt_renderer.h"

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>

#include "renderer_helper.h"
#include "../utils/sampler.h"

namespace spica {

    namespace {

        struct PrimarySample {
            int modifiedTime;
            double value;

            explicit PrimarySample(double randVal = 0.0) 
                : modifiedTime(0)
                , value(randVal)
            {
            }
        };

        struct KelemenMLT {
        private:
            static const int num_init_primary_samples = 128;

        public:            
            int global_time;
            int large_step;
            int large_step_time;
            int used_rand_coords;
            Random rng;

            std::vector<PrimarySample> primary_samples;
            std::stack<PrimarySample> primary_samples_stack;  // Stack for roll back to the previous primary samples

            explicit KelemenMLT(int seed = -1) 
                : global_time(0)
                , large_step(0)
                , large_step_time(0)
                , used_rand_coords(0)
                , rng(seed)
                , primary_samples()
                , primary_samples_stack() 
            {
                primary_samples.resize(num_init_primary_samples);
                for (int i = 0; i < num_init_primary_samples; i++) {
                    primary_samples[i].value = rng.nextReal();
                }
            }

            void initUsedRandCoords() {
                used_rand_coords = 0;
            }

            inline double nextSample() {
                if (primary_samples.size() <= used_rand_coords) {
                    const int nextSize = static_cast<int>(primary_samples.size() * 1.5);
                    while (primary_samples.size() < nextSize) {
                        primary_samples.push_back(PrimarySample(rng.nextReal()));
                    }
                }

                if (primary_samples[used_rand_coords].modifiedTime < global_time) {
                    if (large_step > 0) {
                        primary_samples_stack.push(primary_samples[used_rand_coords]);
                        primary_samples[used_rand_coords].modifiedTime = global_time;
                        primary_samples[used_rand_coords].value = rng.nextReal();
                    } else {
                        if (primary_samples[used_rand_coords].modifiedTime < large_step_time) {
                            primary_samples[used_rand_coords].modifiedTime = large_step_time;
                            primary_samples[used_rand_coords].value = rng.nextReal();
                        }

                        while (primary_samples[used_rand_coords].modifiedTime < global_time - 1) {
                            primary_samples[used_rand_coords].value = mutate(primary_samples[used_rand_coords].value);
                            primary_samples[used_rand_coords].modifiedTime++;
                        }

                        primary_samples_stack.push(primary_samples[used_rand_coords]);
                        primary_samples[used_rand_coords].value = mutate(primary_samples[used_rand_coords].value);
                        primary_samples[used_rand_coords].modifiedTime = global_time;
                    }
                }
                used_rand_coords++;
                return primary_samples[used_rand_coords - 1].value;
            }

        private:
            inline double mutate(const double x) {
                const double r = rng.nextReal();
                const double s1 = 1.0 / 512.0;
                const double s2 = 1.0 / 16.0;
                const double dx = s1 / (s1 / s2 + std::abs(2.0 * r - 1.0)) - s1 / (s1 / s2 + 1.0);
                if (r < 0.5) {
                    const double x1 = x + dx;
                    return (x1 < 1.0) ? x1 : x1 - 1.0;
                } else {
                    const double x1 = x - dx;
                    return (x1 < 0.0) ? x1 + 1.0 : x1;
                }
            }

        };

        Color direct_radiance_sample(const Scene& scene, const Vector3& v0, const Vector3& normal, const int id, KelemenMLT& mlt) {
            const Primitive* light = scene.get(scene.lightID());
            const Material& lightMtrl = scene.getMaterial(scene.lightID());

            const double r1Light = mlt.nextSample();
            const double r2Light = mlt.nextSample();
            Vector3 light_pos, light_normal;
            sampler::on(light, &light_pos, &light_normal, r1Light, r2Light);

            const Vector3 v_to_l = light_pos - v0;
            const Vector3 light_dir = v_to_l.normalized();
            const double dist2 = v_to_l.squaredNorm();
            const double dot0 = normal.dot(light_dir);
            const double dot1 = light_normal.dot(-1.0 * light_dir);

            if (dot0 >= 0.0 && dot1 >= 0.0) {
                const double G = dot0 * dot1 / dist2;
                Intersection intersection;
                if (scene.intersect(Ray(v0, light_dir), intersection) && intersection.objectId() == scene.lightID()) {
                    const Material& mtrl = scene.getMaterial(id);
                    return Color(mtrl.color.multiply(lightMtrl.emission) * (1.0 / PI) * G * light->area());
                }
            }
            return Color(0.0, 0.0, 0.0);
        }

        Color radiance(const Scene& scene, const Ray& ray, const int depth, const int maxDepth, KelemenMLT& mlt) {
            Intersection isect;
            if (!scene.intersect(ray, isect)) {
                return scene.envmap().sampleFromDir(ray.direction());
            }

            const Material& mtrl = scene.getMaterial(isect.objectId());
            const Hitpoint& hitpoint = isect.hitpoint();
            const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            double roulette = std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));

            if (depth > maxDepth) {
                if (mlt.nextSample() >= roulette) {
                    return Color(0.0, 0.0, 0.0);
                }
            } else {
                roulette = 1.0;
            }

            if (mtrl.reftype == REFLECTION_DIFFUSE) {
                if (isect.objectId() != scene.lightID()) {
                    const int shadow_ray = 1;
                    Vector3 direct_light;
                    for (int i = 0; i < shadow_ray; i++) {
                        direct_light += direct_radiance_sample(scene, hitpoint.position(), orientNormal, isect.objectId(), mlt) / shadow_ray;
                    }

                    Vector3 nextDir;
                    const double r1 = mlt.nextSample();
                    const double r2 = mlt.nextSample();
                    sampler::onHemisphere(orientNormal, &nextDir, r1, r2);
                    const Color nextBounceColor = radiance(scene, Ray(hitpoint.position(), nextDir), depth + 1, maxDepth, mlt);
                    return Color((direct_light + mtrl.color.multiply(nextBounceColor)) / roulette);
                } else if (depth == 0) {
                    return mtrl.emission;
                } else {
                    return Color(0.0, 0.0, 0.0);
                }
            } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                Intersection light_intersect;
                Ray reflection_ray = Ray(hitpoint.position(), Vector3::reflect(ray.direction(), hitpoint.normal()));
                Vector3 direct_light;
                if (scene.intersect(reflection_ray, light_intersect) && light_intersect.objectId() == scene.lightID()) {
                    direct_light = scene.getMaterial(scene.lightID()).emission;
                }
                const Color next_bounce_color = radiance(scene, reflection_ray, depth + 1, maxDepth, mlt);
                return Color((direct_light + mtrl.color.multiply(next_bounce_color)) / roulette);
            } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                Intersection light_intersect;
                Ray reflectRay = Ray(hitpoint.position(), Vector3::reflect(ray.direction(), hitpoint.normal()));
                
                Vector3 direct_light;
                if (scene.intersect(reflectRay, light_intersect) && light_intersect.objectId() == scene.lightID()) {
                    direct_light = scene.getMaterial(scene.lightID()).emission;
                }

                bool isIncoming = Vector3::dot(hitpoint.normal(), orientNormal) > 0.0;

                Vector3 reflectDir, refractDir;
                double fresnelRe, fresnelTr;
                if (helper::isTotalRef(isIncoming, hitpoint.position(), ray.direction(), hitpoint.normal(), orientNormal, &reflectDir, &refractDir, &fresnelRe, &fresnelTr)) {
                    // Total reflection
                    const Color next_bounce_color = radiance(scene, reflectRay, depth + 1, maxDepth, mlt);
                    return Color((direct_light + mtrl.color.multiply(next_bounce_color)) / roulette);
                }

                Ray refractRay = Ray(hitpoint.position(), refractDir);
                Intersection light_intersect_refract;
                
                Vector3 direct_light_refraction;
                if (scene.intersect(refractRay, light_intersect_refract) && light_intersect_refract.objectId() == scene.lightID()) {
                    direct_light_refraction = scene.getMaterial(scene.lightID()).emission;
                }

                const double probability = 0.25 + 0.5 * fresnelRe;

                if (depth > 2) {
                    if (mlt.nextSample() < probability) {
                        const Color nextBounceColor = radiance(scene, reflectRay, depth + 1, maxDepth, mlt);
                        return Color(mtrl.color.multiply(fresnelRe * (direct_light + nextBounceColor)) / (probability * roulette));
                    } else {
                        const Color next_bounce_color = radiance(scene, refractRay, depth + 1, maxDepth, mlt);
                        return Color(mtrl.color.multiply(fresnelTr * (direct_light_refraction + next_bounce_color)) / ((1.0 - probability) * roulette));
                    }
                } else {
                    const Color nextBounceColorRe = radiance(scene, reflectRay, depth + 1, maxDepth, mlt);
                    const Color nextBounceColorTr = radiance(scene, refractRay, depth + 1, maxDepth, mlt);
                    const Color nextBounceColor = Color(fresnelRe * (direct_light + nextBounceColorRe) + fresnelTr * (direct_light_refraction + nextBounceColorTr));
                    return Color(mtrl.color.multiply(nextBounceColor) / roulette);
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
            double randnums[4];
            for (int i = 0; i < 4; i++) {
                randnums[i] = mlt.nextSample();
            }
            CameraSample camSample = camera.sample(x, y, randnums);

            const Ray ray = camSample.generateRay();
            Color c = radiance(scene, ray, 0, maxDepth, mlt);
            weight *= camera.sensitivity() / camSample.totalPdf();

            return PathSample(x, y, c, weight);
        }

    }  // anonymous namespace

    MLTRenderer::MLTRenderer(spica::Image* image)
        : _image(image)
    {
    }

    MLTRenderer::~MLTRenderer()
    {
    }

    void MLTRenderer::render(const Scene& scene, const Camera& camera, Random& rng, int numMLT, int numMutate, int maxDepth) {
        const int width  = camera.imageW();
        const int height = camera.imageH();
        Image* buffer = new Image[OMP_NUM_CORE];
        Random* rand = new Random[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {
            buffer[i] = Image(width, height);
            rand[i] = Random(rng.nextInt());
        }

        const int taskPerThread = (numMLT + OMP_NUM_CORE - 1) / OMP_NUM_CORE;

        bool isAllocImageInside = false;
        if (_image == NULL) {
            _image = new spica::Image(width, height);
            isAllocImageInside = true;
        }
        _image->resize(width, height);

        for (int t = 0; t < taskPerThread; t++) {
            ompfor (int threadID = 0; threadID < OMP_NUM_CORE; threadID++) {
                RandomSeq rseq;
                rand[threadID].requestSamples(rseq, 2);

                KelemenMLT kelemenMlt(threadID);

                const int seed_path_max = std::max(width, height);

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

                    sumI += sample.F.luminance();
                    seed_paths[i] = sample;
                }

                // Compute first path
                const double rnd = rseq.next() * sumI;
                int selected_path = 0;
                double accumulated_importance = 0.0;
                for (int i = 0; i < seed_path_max; i++) {
                    accumulated_importance += seed_paths[i].F.luminance();
                    if (accumulated_importance >= rnd) {
                        selected_path = i;
                        break;
                    }
                }

                // Mutation
                const double b = sumI / seed_path_max;
                const double p_large = 0.5;
                int accept = 0;
                int reject = 0;
                PathSample old_path = seed_paths[selected_path];
                int progress = 0;
                for (int i = 0; i < numMutate; i++) {
                    if ((i + 1) % (numMutate / 10) == 0) {
                        progress += 10;
                        printf("Thread No.%d: %3d %%, Accept: %8d, Reject %8d, Rate: %7.4f %%\r", omp_thread_id() + 1, progress, accept, reject, 100.0 * accept / (accept + reject));
                    }

                    rand[threadID].requestSamples(rseq, 2);

                    kelemenMlt.large_step = rseq.next() < p_large ? 1 : 0;
                    kelemenMlt.initUsedRandCoords();
                    PathSample new_path = generateNewPath(scene, camera, kelemenMlt, -1, -1, maxDepth);

                    double a = std::min(1.0, new_path.F.luminance() / old_path.F.luminance());
                    const double new_path_weight = (a + kelemenMlt.large_step) / (new_path.F.luminance() / b + p_large) / numMutate;
                    const double old_path_weight = (1.0 - a) / (old_path.F.luminance() / b + p_large) / numMutate;

                    buffer[threadID].pixel(new_path.x, new_path.y) += new_path.weight * new_path_weight * new_path.F;
                    buffer[threadID].pixel(old_path.x, old_path.y) += old_path.weight * old_path_weight * old_path.F;
                
                    if (rseq.next() < a) {
                        // Accept
                        accept++;
                        old_path = new_path;
                        if (kelemenMlt.large_step) {
                            kelemenMlt.large_step_time = kelemenMlt.global_time;
                        }
                        kelemenMlt.global_time++;
                        while (!kelemenMlt.primary_samples_stack.empty()) {
                            kelemenMlt.primary_samples_stack.pop();
                        }
                    } else {
                        // Reject
                        reject++;
                        int idx = kelemenMlt.used_rand_coords - 1;
                        while (!kelemenMlt.primary_samples_stack.empty()) {
                            kelemenMlt.primary_samples[idx--] = kelemenMlt.primary_samples_stack.top();
                            kelemenMlt.primary_samples_stack.pop();
                        }
                    }
                }
            }

            const int usedSamples = OMP_NUM_CORE * (t + 1);
            _image->fill(Color(0.0, 0.0, 0.0));
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    for (int k = 0; k < OMP_NUM_CORE; k++) {
                        _image->pixel(width - x - 1, y) += buffer[k](x, y) / usedSamples;
                    }
                }
            }

            char filename[256];
            sprintf(filename, "mlt_%03d.bmp", usedSamples);
            _image->gamma(1.7, true);
            _image->saveBMP(filename);
        }
        printf("\nFinish\n");

        // Release memory
        delete[] buffer;
        delete[] rand;
        if (isAllocImageInside) {
            delete _image;
        }
    }

}  // namespace spica
