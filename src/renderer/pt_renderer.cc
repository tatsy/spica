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
#include "renderer_helper.h"

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
                for (int i = 0; i < samplePerPixel; i++) {
                    image.pixel(width - x - 1, y) += executePT(scene, camera, x, y, rng) / samplePerPixel;
                }
            }

            omplock {
                processed++;
                printf("%6.2f %% processed...\n", 100.0 * processed / height);
            }
        }
        
        image.saveBMP("path_trace.bmp");

        return 0;
    }

    Color PTRenderer::executePT(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, const Random& rng) {
        Vector3 posOnSensor;        // Position on the image sensor
        Vector3 posOnObjplane;      // Position on the object plane
        Vector3 posOnLens;          // Position on the lens
        double  pImage, pLens;      // Sampling probability on image sensor and lens

        camera.samplePoints(pixelX, pixelY, rng, posOnSensor, posOnObjplane, posOnLens, pImage, pLens);
        const Ray ray(posOnLens, Vector3::normalize(posOnObjplane - posOnLens));

        Vector3 lens2sensor = posOnSensor - posOnLens;
        const double cosine = Vector3::dot(camera.direction(), lens2sensor.normalized());
        const double weight = cosine * cosine / lens2sensor.squaredNorm();

        return helper::radiance(scene, ray, rng, 0) * (weight * camera.sensitivity() / (pImage * pLens));
    }

    

}  // namespace spica
