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

    PathTracingRenderer::PathTracingRenderer()
    {
    }

    PathTracingRenderer::~PathTracingRenderer() 
    {
    }

    void PathTracingRenderer::render(const Scene& scene, const Camera& camera, const int samplePerPixel, RandomType randType) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        RandomBase* rand = NULL;
        switch (randType) {
        case PSEUDO_RANDOM_TWISTER:
            printf("Use pseudo random numbers (Twister)\n");
            rand = new Random();
            break;

        case QUASI_MONTE_CARLO:
            printf("Use quasi random numbers (Halton)\n");
            rand = new Halton();
            break;

        default:
            msg_assert(false, "Unknown random number generator type!!");
        }

        // Vectors spanning screen
        Image buffer(width, height);
        int processed = 0;
        
        for (int i = 0; i < samplePerPixel; i++) {
            ompfor (int y = 0; y < height; y++) {
                RandomSeq rseq;
                for (int x = 0; x < width; x++) {                    
                    omplock {
                        rand->requestSamples(rseq, 200);
                    }

                    buffer.pixel(width - x - 1, y) += executePathTracing(scene, camera, x, y, rseq);
                }
            
                omplock {
                    processed++;
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

            omplock {
                printf("  %6.2f %% processed -> %s\n", 100.0 * processed / (height * samplePerPixel), filename);
            }
        }
        printf("\nFinish!!\n");

        delete rand;
    }

    Color PathTracingRenderer::executePathTracing(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, RandomSeq& rseq) {
        Vector3 posOnSensor;        // Position on the image sensor
        Vector3 posOnObjplane;      // Position on the object plane
        Vector3 posOnLens;          // Position on the lens
        double  pImage, pLens;      // Sampling probability on image sensor and lens

        CameraSample camSample = camera.sample(pixelX, pixelY, rseq);
        const Ray ray = camSample.generateRay();

        return helper::radiance(scene, ray, rseq, 0) * (camera.sensitivity() / camSample.totalPdf());
    }

}  // namespace spica
