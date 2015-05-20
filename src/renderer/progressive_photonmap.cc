#define SPICA_PROGRESSIVE_PHOTONMAP_EXPORT
#include "progressive_photonmap.h"

#include <algorithm>

#include "renderer_helper.h"
#include "../utils/sampler.h"

namespace spica {

    namespace {

        struct HitpointPPM {
            Vector3 position;
            Vector3 normal;
            Vector3 inDir;
            int x, y;
            Color weight;
            double radius;
            int numPhotons;
            Color accumFlux;
        };

    }  // anonymous namespace

    PPMRenderer::PPMRenderer()
    {
    }

    PPMRenderer::PPMRenderer(const PPMRenderer& renderer)
    {
    }

    PPMRenderer::~PPMRenderer()
    {
    }

    int PPMRenderer::render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel) const {
        const int width = camera.imageW();
        const int height = camera.imageH();
        Image image(width, height);

        
        
        int proc = 0;
        for (int i = 0; i < samplePerPixel; i++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    // image.pixel(width - x - 1, y) += executePT(scene, camera);
                }

                proc += 1;
                printf("%6.2f %% processed ...\n", 100.0 * proc / (samplePerPixel * height));
            }
        }
    }

    void PPMRenderer::traceRays(const Scene& scene, const Camera& camera, const Random& rng) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        
        // Generate a ray to cast
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                executePathTracing(scene, camera, rng, x, y);
            }
        }
    }

    void PPMRenderer::tracePhotons(const Scene& scene, const Random& rng, const int numPhotons) {
        for (int pid = 0; pid < numPhotons; pid++) {
            // Sample point on light
            const int lightID = scene.lightID();
            const Primitive* light = scene.get(lightID);

            Vector3 posOnLight, normalOnLight;
            sampler::on(light, &posOnLight, &normalOnLight);
            
            // Compute flux
            double pdfAreaOnLight = 1.0 / light->area();
            double totalPdfArea = pdfAreaOnLight;
            Color currentFlux = light->area() * scene.getMaterial(lightID).emission / numPhotons;

            // Prepare ray
            Vector3 nextDir;
            sampler::onHemisphere(normalOnLight, &nextDir);
            Ray currentRay(posOnLight, nextDir);
            Vector3 prevNormal = normalOnLight;

            // Shooting photons
            for (;;) {
                // Remove photons with zero flux
                if (std::max(currentFlux.red(), std::max(currentFlux.green(), currentFlux.blue())) < 0.0) {
                    break;
                }

                // Intersection check
                Intersection isect;
                bool isHit = scene.intersect(currentRay, isect);
                if (!isHit) {
                    break;
                }

                const int objectID = isect.objectId();
                const Material& mtrl = scene.getMaterial(objectID);
                const Hitpoint& hitpoint = isect.hitpoint();
                const Vector3 orientNormal = Vector3::dot(hitpoint.normal(), currentRay.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

                if (mtrl.reftype == REFLECTION_DIFFUSE) {
                    // Photon reaches diffuse surface. Update hitpoints.
                } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                                    
                } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                
                }
            }
        }
    }

    void PPMRenderer::executePathTracing(const Scene& scene, const Camera& camera, const Random& rng, const int imageX, const int imageY) {
        Vector3 posOnSensor, posOnObjplane, posOnLens;
        double pImage, pLens;
        Intersection isect;

        // Generate a path to trace
        camera.samplePoints(imageX, imageY, rng, posOnSensor, posOnObjplane, posOnLens, pImage, pLens);
        Ray ray(posOnLens, (posOnObjplane - posOnLens).normalized());

        Color throughput(1.0, 1.0, 1.0);
        for (int bounce = 0; ; bounce++) {
            if (scene.intersect(ray, isect)) {
                throughput = throughput.cwiseMultiply(scene.bgColor());
                break;
            }

            int objectID = isect.objectId();
            const Hitpoint& hitpoint = isect.hitpoint();
            const Material& mtrl = scene.getMaterial(objectID);

            Vector3 orientNormal = Vector3::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            // Russian roulette
            double roulette = std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));
            if (bounce > bounceLimit) {
                if (rng.nextReal() > roulette) {
                    break;
                }
            } else {
                roulette = 1.0;
            }

            if (mtrl.reftype == REFLECTION_DIFFUSE) {
                // Ray hits diffuse object, then store point

            } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                Vector3 nextDir = Vector3::reflect(ray.direction(), orientNormal);
                ray = Ray(hitpoint.position(), nextDir);
                throughput = throughput.cwiseMultiply(mtrl.color);
            } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                bool isIncoming = Vector3::dot(hitpoint.normal(), orientNormal) > 0.0;
                Vector3 reflectDir, refractDir;
                double fresnelRe, fresnelTr;
                if (helper::isTotalRef(isIncoming, hitpoint.position(), ray.direction(), hitpoint.normal(), orientNormal, &reflectDir, &refractDir, &fresnelRe, &fresnelTr)) {
                    // Total reflection

                } else {
                    // Trace either reflection or refraction ray with probability
                    const double probability = 0.25 + 0.5 * fresnelRe;
                    if (rng.nextReal() < fresnelRe) {
                        // Reflection

                    } else {
                        // Transmit

                    }
                }
            }
        }
    }

}
