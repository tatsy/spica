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

    void PTRenderer::render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        // Vectors spanning screen
        Image buffer(width, height);
        int processed = 0;
        
        for (int i = 0; i < samplePerPixel; i++) {
            ompfor (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    buffer.pixel(width - x - 1, y) += executePathTracing(scene, camera, x, y, rng);
                }
            
                omplock {
                    processed++;
                    printf("%6.2f %% processed...\n", 100.0 * processed / (height * samplePerPixel));
                }
            }

            char filename[256];
            Image image(width, height);
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    image.pixel(x, y) = buffer(x, y) / (i + 1);
                }
            }
            sprintf(filename, "pathtrace_%03d.bmp", i + 1);
            image.saveBMP(filename);
        }
    }

    Color PTRenderer::executePathTracing(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, const Random& rng) {
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

    void PTRenderer::renderQMC(const Scene& scene, const Camera& camera, const Halton& halton, const int samplePerPixel) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        Image buffer(width, height);
        int proc = 0;

        for (int i = 0; i < samplePerPixel; i++) {
            ompfor (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    const int sampleID = (i * height + y) * width + x; // (y * width + x) * samplePerPixel + i;
                    buffer.pixel(width - x - 1, y) += executePathTracingQMC(scene, camera, x, y, halton, sampleID);
                }

                omplock {
                    proc++;
                    printf("%6.2f %% processed ...\n", 100.0 * proc / (height * samplePerPixel));
                }
            }

            char filename[256];
            Image image(width, height);
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    image.pixel(x, y) = buffer(x, y) / (i + 1);
                }
            }
            sprintf(filename, "pathtrace_qmc_%03d.bmp", i + 1);
            image.saveBMP(filename);
        }
    }

    Color PTRenderer::executePathTracingQMC(const Scene& scene, const Camera& camera, const double imageX, const double imageY, const Halton& halton, const int sampleID) {
        CameraSample camSample = camera.sample(imageX, imageY, halton, sampleID);
        const Ray ray = camSample.generateRay();
        return helper::radiance(scene, ray, halton, sampleID, 0) * camera.sensitivity() / camSample.totalPdf();          
    }

    

}  // namespace spica
