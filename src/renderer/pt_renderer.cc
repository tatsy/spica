#define SPICA_PT_RENDERER_EXPORT
#include "pt_renderer.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "../utils/common.h"
#include "../utils/vector3.h"
#include "../utils/sampler.h"
#include "../utils/image.h"
#include "material.h"
#include "scene.h"

namespace spica {

    PTRenderer::PTRenderer()
    {
    }

    PTRenderer::PTRenderer(const PTRenderer& renderer)
    {
    }

    PTRenderer::~PTRenderer() 
    {
    }

    PTRenderer& PTRenderer::operator=(const PTRenderer& renderer) {
        return *this;
    }

    int PTRenderer::render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        // Vectors spanning screen
        Image image(width, height);
        int processed = 0;
        
        ompfor (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                image.pixel(width - x - 1, y) = executePT(scene, camera, x, y, rng, samplePerPixel);
            }

            omplock
            {
                processed++;
                printf("%6.2f %% processed...\n", 100.0 * processed / height);
            }
        }
        
        image.savePPM("simplept.ppm");

        return 0;
    }

    Color PTRenderer::executePT(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, const Random& rng, const int samplePerPixel) {
        Vector3 posOnSensor;        // Position on the image sensor
        Vector3 posOnObjplane;      // Position on the object plane
        Vector3 posOnLens;          // Position on the lens
        double  pImage, pLens;      // Sampling probability on image sensor and lens

        Color throughput(0.0, 0.0, 0.0);
        for (int i = 0; i < samplePerPixel; i++) {
            camera.samplePoints(pixelX, pixelY, rng, posOnSensor, posOnObjplane, posOnLens, pImage, pLens);
            const Ray ray(posOnLens, Vector3::normalize(posOnObjplane - posOnLens));
            throughput += radiance(scene, ray, rng, 0) / (pImage * pLens);                    
        }
        return throughput / samplePerPixel;
    }

    Color PTRenderer::radiance(const Scene& scene, const Ray& ray, const Random& rng, const int depth, const int depthLimit, const int maxDepth) {
        Intersection isect;

        // NOT intersect the scene
        if (!scene.intersect(ray, isect)) {
            return scene.bgColor();
        }

        const Material& mtrl = scene.getMaterial(isect.objectId());
        const Hitpoint& hitpoint = isect.hitpoint();
        const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

        double roulette =std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));

        if (depth > depthLimit) {
            roulette *= pow(0.5, depth - depthLimit);
        }

        if (depth > maxDepth) {
            if (rng.randReal() > roulette) {
                return mtrl.emission;
            }
        } else {
            roulette = 1.0;
        }

        Color incomingRad;
        Color weight = Color(1.0, 0.0, 0.0);

        if (mtrl.reftype == REFLECTION_DIFFUSE) {
            Vector3 nextDir;
            sampler::onHemisphere(orientNormal, &nextDir);
            incomingRad = radiance(scene, Ray(hitpoint.position(), nextDir), rng, depth + 1);
            weight = mtrl.color / roulette;
        } else if (mtrl.reftype == REFLECTION_SPECULAR) {
            Vector3 nextDir = ray.direction() - (2.0 * hitpoint.normal().dot(ray.direction())) * hitpoint.normal();
            incomingRad = radiance(scene, Ray(hitpoint.position(), nextDir), rng, depth + 1);
            weight = mtrl.color / roulette;
        } else if (mtrl.reftype == REFLECTION_REFRACTION) {
            Vector3 reflectDir = ray.direction() - (2.0 * hitpoint.normal().dot(ray.direction())) * hitpoint.normal();
            const Ray reflectRay = Ray(hitpoint.position(), reflectDir);

            // Incoming or outgoing
            const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

            // Snell's rule
            const double nc = IOR_VACCUM;
            const double nt = IOR_OBJECT;
            const double nnt = isIncoming ? nc / nt : nt / nc;
            const double ddn = ray.direction().dot(orientNormal);
            const double cos2t = 1.0 - nnt * nnt * (1.0  - ddn * ddn);

            if (cos2t < 0.0) { // Total reflection
                incomingRad = radiance(scene, reflectRay, rng, depth + 1);
                weight = mtrl.color / roulette;
            } else {
                Vector3 refractDir = (ray.direction() * nnt - hitpoint.normal() * (isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalized();
                const Ray refractRay = Ray(hitpoint.position(), refractDir);

                // Schlick's approximation of Fresnel coefficient
                const double a = nt - nc;
                const double b = nt + nc;
                const double R0 = (a * a) / (b * b);

                const double c = 1.0 - (isIncoming ? -ddn : - refractDir.dot(orientNormal));
                const double Re = R0 + (1.0 - R0) * pow(c, 5.0);
                const double nnt2 = pow(isIncoming ? nc / nt : nt / nc, 2.0);
                const double Tr = (1.0 - Re) * nnt2;

                const double prob = 0.25 + 0.5 * Re;
                if (depth > 2) {
                    if (rng.randReal() < prob) {
                        incomingRad = radiance(scene, reflectRay, rng, depth + 1) * Re;
                        weight = mtrl.color / (prob * roulette);
                    } else {
                        incomingRad = radiance(scene, refractRay, rng, depth + 1) * Tr;
                        weight = mtrl.color / ((1.0 - prob) * roulette);
                    }
                } else {
                    incomingRad = radiance(scene, reflectRay, rng, depth + 1) * Re + radiance(scene, refractRay, rng, depth + 1) * Tr;
                    weight = mtrl.color / roulette;
                }
            }
        } else if (mtrl.reftype == REFLECTION_SUBSURFACE) {
            msg_assert(false, "Future implementation");
        }

        return mtrl.emission + weight.cwiseMultiply(incomingRad);
    }

}  // namespace spica
