#define SPICA_API_EXPORT
#include "mlt.h"

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>

#include "renderer_helper.h"

#include "../core/sampling.h"

#include "../image/tmo.h"

#include "../bsdf/bsdf.h"

#include "../random/random_base.h"

#include "render_parameters.h"

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

        struct KelemenMLT : public RandomBase {
        private:
            static const int num_init_primary_samples = 128;

        public:            
            int global_time;
            int large_step;
            int large_step_time;
            int used_rand_coords;
            Random rng;

            std::vector<PrimarySample> primary_samples;
            std::stack<PrimarySample> primary_samples_stack;

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

            void request(Stack<double>* rstk, int numRequested) override {
                for (int i = 0; i < numRequested; i++) {
                    rstk->push(nextSample());
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

        Spectrum radiance(const Scene& scene, const Ray& ray, const RenderParameters& params, int bounces, KelemenMLT& mlt) {
            Intersection isect;
            if (!scene.intersect(ray, &isect)) {
                return Spectrum{};
            }

            const int    objectID = isect.objectID();
            const BSDF&  bsdf     = scene.getBsdf(objectID);
            const Spectrum& refl     = isect.color();

            double roulette = max3(refl.red(), refl.green(), refl.blue());
            if (bounces > params.bounceStartRoulette()) {
                if (mlt.nextSample() >= roulette) {
                    return Spectrum{};
                }
            } else {
                roulette = 1.0;
            }

            if (!scene.isLightCheck(isect.objectID())) {
                Stack<double> rstk;
                mlt.request(&rstk, 3);
                Spectrum dlight = helper::directLight(scene, isect.position(), ray.direction(), isect.normal(), bsdf, rstk);

                double pdf = 1.0;
                Vector3D nextdir;
                bsdf.sample(ray.direction(), isect.normal(), mlt.nextSample(), mlt.nextSample(), &nextdir, &pdf);

                Ray nextRay(isect.position(), nextdir);
                Spectrum nextrad = radiance(scene, nextRay, params, bounces + 1, mlt);
                return Spectrum(refl * (dlight + nextrad / pdf) / roulette);
            } else if (bounces == 0) {
                return scene.directLight(ray.direction());
            }
            return Spectrum(0.0, 0.0, 0.0);
        }

        struct PathSample {
            int x, y;
            Spectrum F;
            double weight;
            PathSample(const int x_ = 0, const int y_ = 0, const Spectrum& F_ = Spectrum(), const double weight_ = 1.0)
                : x(x_)
                , y(y_)
                , F(F_)
                , weight(weight_)
            {
            }
        };

        PathSample generateNewPath(const Scene& scene, const Camera& camera, const RenderParameters& params, KelemenMLT& mlt, int x, int y, int maxDepth) {
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
            Stack<double> stk;
            for (int i = 0; i < 4; i++) {
                stk.push(mlt.nextSample());
            }

            CameraSample camSample = camera.sample(x, y, stk);

            const Ray ray = camSample.ray();
            Spectrum c = radiance(scene, ray, params, 0, mlt);
            weight *= camera.sensitivity() / camSample.pdf();

            return PathSample(x, y, c, weight);
        }

    }  // anonymous namespace

    MLTRenderer::MLTRenderer()
        : IRenderer{RendererType::Metropolis} {
    }

    MLTRenderer::~MLTRenderer() {
    }

    void MLTRenderer::render(const Scene& scene, const Camera& camera, 
                             const RenderParameters& params) {
        const int width  = camera.imageW();
        const int height = camera.imageH();
        
        const int numMLT = params.samplePerPixel();
        const int numMutate = width * height;
        
        Image* buffer = new Image[kNumThreads];
        Random* rand = new Random[kNumThreads];
        for (int i = 0; i < kNumThreads; i++) {
            buffer[i] = Image(width, height);
            rand[i] = Random(i);
        }

        const int taskPerThread = (numMLT + kNumThreads- 1) / kNumThreads;

        _result.resize(width, height);
        const int seed_path_max = std::max(width, height);
        for (int t = 0; t < taskPerThread; t++) {
            ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
                Stack<double> rstk;
                rand[threadID].request(&rstk, 2);

                KelemenMLT kelemenMlt(threadID);

                std::vector<PathSample> seed_paths(seed_path_max);
                double sumI = 0.0;
                kelemenMlt.large_step = 1;
                for (int i = 0; i < seed_path_max; i++) {
                    kelemenMlt.initUsedRandCoords();
                    PathSample sample = generateNewPath(scene, camera, params, kelemenMlt, -1, -1, params.bounceLimit());
                    kelemenMlt.global_time++;
                    while (!kelemenMlt.primary_samples_stack.empty()) {
                        kelemenMlt.primary_samples_stack.pop();
                    }

                    sumI += sample.F.luminance();
                    seed_paths[i] = sample;
                }

                // Compute first path
                const double rnd = rstk.pop() * sumI;
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
                        printf("Thread No.%d: %3d %%, Accept: %8d, Reject %8d, Rate: %7.4f %%\n", omp_thread_id() + 1, progress, accept, reject, 100.0 * accept / (accept + reject));
                    }

                    rand[threadID].request(&rstk, 2);

                    kelemenMlt.large_step = rstk.pop() < p_large ? 1 : 0;
                    kelemenMlt.initUsedRandCoords();
                    PathSample new_path = generateNewPath(scene, camera, params, kelemenMlt, -1, -1, params.bounceLimit());

                    double a = std::min(1.0, new_path.F.luminance() / old_path.F.luminance());
                    const double new_path_weight = (a + kelemenMlt.large_step) / (new_path.F.luminance() / b + p_large) / numMutate;
                    const double old_path_weight = (1.0 - a) / (old_path.F.luminance() / b + p_large) / numMutate;

                    buffer[threadID].pixel(new_path.x, new_path.y) += new_path.weight * new_path_weight * new_path.F;
                    buffer[threadID].pixel(old_path.x, old_path.y) += old_path.weight * old_path_weight * old_path.F;
                
                    if (rstk.pop() < a) {
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

            const int usedSamples = kNumThreads * (t + 1);
            _result.fill(Spectrum{});
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    for (int k = 0; k < kNumThreads; k++) {
                        _result.pixel(width - x - 1, y) += buffer[k](x, y) / usedSamples;
                    }
                }
            }

            char filename[256];
            sprintf(filename, params.saveFilenameFormat().c_str(), usedSamples);
            _result = GammaTmo(2.2).apply(_result);
            _result.save(filename);
        }
        printf("\nFinish\n");

        // Release memory
        delete[] buffer;
        delete[] rand;
    }

}  // namespace spica
