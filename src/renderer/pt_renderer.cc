#define SPICA_PT_RENDERER_EXPORT
#include "pt_renderer.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "../utils/common.h"
#include "../utils/vector3d.h"
#include "../utils/sampler.h"
#include "../utils/image.h"
#include "scene.h"
#include "renderer_helper.h"

namespace spica {

    namespace {

        Color executePathTracing(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, RandomSeq& rseq) {
            Vector3D posOnSensor;        // Position on the image sensor
            Vector3D posOnObjplane;      // Position on the object plane
            Vector3D posOnLens;          // Position on the lens
            double  pImage, pLens;      // Sampling probability on image sensor and lens

            CameraSample camSample = camera.sample(pixelX, pixelY, rseq);
            const Ray ray = camSample.generateRay();

            return Color(helper::radiance(scene, ray, rseq, 0) * (camera.sensitivity() / camSample.totalPdf()));
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

        // Prepare random number generators
        RandomBase** rand = new RandomBase*[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {
            switch (randType) {
            case PSEUDO_RANDOM_TWISTER:
                printf("Use pseudo random numbers (Twister)\n");
                rand[i] = new Random(i);
                break;

            case QUASI_MONTE_CARLO:
                printf("Use quasi random numbers (Halton)\n");
                rand[i] = new Halton(200, true, i);
                break;

            default:
                Assertion(false, "Unknown random number generator type!!");
            }
        }

        // Vectors spanning screen
        Image buffer = Image(width, height);

        // Distribute rendering tasks
        const int taskPerThread = (height + OMP_NUM_CORE - 1) / OMP_NUM_CORE;
        std::vector<std::vector<int> > tasks(OMP_NUM_CORE);
        for (int y = 0; y < height; y++) {
            tasks[y % OMP_NUM_CORE].push_back(y);
        }

        // Allocate image
        bool isAllocImageInside = false;
        if (_image == NULL) {
            _image = new Image();
            isAllocImageInside = true;
        }
        _image->resize(width, height);

        // Trace rays
        int processed = 0;
        buffer.fill(Color::BLACK);
        for (int i = 0; i < samplePerPixel; i++) {
            for (int t = 0; t < taskPerThread; t++) {
                ompfor (int threadID = 0; threadID < OMP_NUM_CORE; threadID++) {
                    if (t < tasks[threadID].size()) {
                        RandomSeq rseq;
                        const int y = tasks[threadID][t];
                        for (int x = 0; x < width; x++) {                    
                            rand[threadID]->requestSamples(rseq, 200);
                            buffer.pixel(width - x - 1, y) += executePathTracing(scene, camera, x, y, rseq);
                        }
                    }
                }
            }

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    _image->pixel(x, y) = buffer(x, y) / (i + 1);
                }
            }

            char filename[256];
            sprintf(filename, "pathtrace_%03d.bmp", i + 1);
            _image->gamma(2.2, true);
            _image->save(filename);

            printf("  %6.2f %%  processed -> %s\r", 100.0 * (i + 1) / samplePerPixel, filename);
        }
        printf("\nFinish!!\n");

        for (int i = 0; i < OMP_NUM_CORE; i++) {
            delete rand[i];
        }
        delete[] rand;

        if (isAllocImageInside) {
            delete _image;
        }
    }

}  // namespace spica
