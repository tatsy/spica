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

    namespace {

        Color executePathTracing(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, RandomSeq& rseq) {
            Vector3 posOnSensor;        // Position on the image sensor
            Vector3 posOnObjplane;      // Position on the object plane
            Vector3 posOnLens;          // Position on the lens
            double  pImage, pLens;      // Sampling probability on image sensor and lens

            CameraSample camSample = camera.sample(pixelX, pixelY, rseq);
            const Ray ray = camSample.generateRay();

            return helper::radiance(scene, ray, rseq, 0) * (camera.sensitivity() / camSample.totalPdf());
        }

    }   // anonymous namespace

    PathTracingRenderer::PathTracingRenderer(spica::Image* image)
        : _image(image)
    {
    }

    PathTracingRenderer::~PathTracingRenderer() 
    {
    }

    void PathTracingRenderer::render(const Scene& scene, const Camera& camera, const int samplePerPixel, RandomType randType) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        RandomBase** rand = new RandomBase*[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {
            switch (randType) {
            case PSEUDO_RANDOM_TWISTER:
                printf("Use pseudo random numbers (Twister)\n");
                rand[i] = new Random();
                break;

            case QUASI_MONTE_CARLO:
                printf("Use quasi random numbers (Halton)\n");
                rand[i] = new Halton(200, true, i);
                break;

            default:
                msg_assert(false, "Unknown random number generator type!!");
            }
        }

        // Vectors spanning screen
        Image* buffer = new Image[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {
            buffer[i] = Image(width, height);
        }

        const int taskPerThread = (samplePerPixel + OMP_NUM_CORE - 1) / OMP_NUM_CORE;
        int processed = 0;        

        bool isAllocImageInside = false;
        if (_image == NULL) {
            _image = new Image();
            isAllocImageInside = true;
        }
        _image->resize(width, height);

        for (int t = 0; t < taskPerThread; t++) {
            ompfor (int threadID = 0; threadID < OMP_NUM_CORE; threadID++) {
                RandomSeq rseq;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {                    
                        rand[threadID]->requestSamples(rseq, 200);
                        buffer[threadID].pixel(width - x - 1, y) += executePathTracing(scene, camera, x, y, rseq);
                    }
                }
            }


            char filename[256];
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    _image->pixel(x, y) = Color(0.0, 0.0, 0.0);
                    for (int k = 0; k < OMP_NUM_CORE; k++) {
                        _image->pixel(x, y) += buffer[k](x, y) / ((t + 1) * OMP_NUM_CORE);
                    }
                }
            }
            sprintf(filename, "pathtrace_%03d.bmp", t + 1);
            _image->saveBMP(filename);

            printf("  %6.2f %%  processed -> %s\r", 100.0 * (t + 1) / taskPerThread, filename);
        }
        printf("\nFinish!!\n");

        for (int i = 0; i < OMP_NUM_CORE; i++) {
            delete rand[i];
        }
        delete[] rand;
        delete[] buffer;

        if (isAllocImageInside) {
            delete _image;
        }
    }

}  // namespace spica
