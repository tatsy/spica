#define SPICA_MLT_RENDERER_EXPORT
#include "mlt_renderer.h"

#include <iostream>
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

                    used_rand_coords++;
                    return primary_samples[used_rand_coords - 1].value;
                }
            }
        };

        Color radiance(const Scene& scene, const Ray& ray, const int depth, const int maxDepth, KelemenMLT& mlt) {
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

		PathSample generate_new_path(const Scene& scene, const Ray& camera, const Vector3& cx, const Vector3& cy, const int width, const int height, KelemenMLT& mlt, int x, int y) {
			double weight = 4.0;
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

			int sx = mlt.nextSample() < 0.5 ? 0 : 1;
			int sy = mlt.nextSample() < 0.5 ? 0 : 1;

			const double r1 = 2.0 * mlt.nextSample();
			const double r2 = 2.0 * mlt.nextSample();
			const double dx = r1 < 1.0 ? sqrt(r1) - 1.0 : 1.0 - sqrt(2.0 - r1);
			const double dy = r2 < 1.0 ? sqrt(r2) - 1.0 : 1.0 - sqrt(2.0 - r2);
			Vector3 dir = cx * (((sx + 0.5 + dx) / 2.0 + x) / width - 0.5) + cy * (((sy + 0.5 + dy) / 2.0 + y) / height - 0.5) + camera.direction();
			const Ray ray = Ray(camera.origin() + camera.direction() * 130.0, dir.normalize());

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

    int MLTRenderer::render(const Scene& scene, const Camera& camera) {
		const int width = camera.imageWidth();
		const int height = camera.imageHeight();
		const int mlt_num = width * height; // TODO: Revise
		Image tmp_image(width, height);
		for (int mi = 0; mi < mlt_num; mi++) {
			KelemenMLT mlt;

			int seed_path_max = width * height;
			if (seed_path_max <= 0) {
				seed_path_max = 1;
			}

			std::vector<PathSample> seed_paths(seed_path_max);
			double sumI = 0.0;
			mlt.large_step = 1;
			for (int i = 0; i < seed_path_max; i++) {
				mlt.initUsedRandCoords();
				PathSample smaple = generate_new_path(scene, camera.lens().normal(), cx, cy, width, height, mlt, -1, -1);
				mlt.global_time++;
				while (!mlt.primary_samples_stack.empty()) {
					mlt.primary_samples_stack.pop();
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
			const int M = mutation;
			int accept = 0;
			int reject = 0;
			PathSample old_path = seed_paths[selected_path];
			int progress = 0;
			for (int i = 0; i < M; i++) {
				if ((i + 1) % (M / 10) == 0) {
					progress += 10;
					std::cout << progress << " % ";
					std::cout << "Accept: " << accept << ", Reject: " << reject;
					std::cout << ", Rate: " << (100.0 * accept / (accept + reject)) << " %" << std::endl;
				}

				mlt.large_step = rng.randReal() < p_large;
				mlt.initUsedRandCoords();
				PathSample new_sample = generate_new_path(scene, camera.lens().normal(), cx, cy, width, height, -1, -1);

				double a = std::min(1.0, luminance(new_path.F) / luminance(old_path.F));
				const double new_path_weight = (a + mlt.large_step) / (luminance(new_path.F) / b + p_large) / M;
				const double old_path_weight = (1.0 - a) / (luminance(old_path.F) / b + p_large) / M;

				tmp_image.pixel(new_path.x, new_path.y) += new_path_weight * new_path_weight * new_path.F;
				tmp_image.pixel(old_path.x, old_path.y) += old_path_weight * old_path_weight * old_path.F;

				if (rng.randReal() < a) {  // Accept
					accept++;
					old_path = new_path;
					if (mlt.large_step) {
						mlt.large_step_time += mlt.global_time;
					}
					mlt.global_time++;
					while (!mlt.primary_samples_stack.empty()) {
						mlt.primary_samples_stack.pop();
					}
				} else {  // Reject
					reject++;
					int idx = mlt.used_rand_coords - 1;
					while (!mlt.primary_samples_stack.empty()) {
						mlt.primary_samples[idx--] = mlt.primary_samples_stack.top();
						mlt.primary_samples_stack.pop();
					}
				}
			}

			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					image.pixel(x, y) += tmp_image.pixel(x, y) / mlt_num;
				}
			}
		}
    }

}  // namespace spica
