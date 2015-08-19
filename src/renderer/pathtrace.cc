#define SPICA_PT_RENDERER_EXPORT
#include "pathtrace.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "../utils/common.h"
#include "../utils/vector3d.h"
#include "../utils/sampler.h"
#include "../utils/image.h"

#include "../random/random_sampler.h"

#include "scene.h"
#include "renderer_helper.h"

namespace spica {

    namespace {

        Color executePathTracing(const Scene& scene, const Camera& camera, const RenderParameters& params, const double pixelX, const double pixelY, Stack<double>& rands) {
            Vector3D posOnSensor;        // Position on the image sensor
            Vector3D posOnObjplane;      // Position on the object plane
            Vector3D posOnLens;          // Position on the lens
            double  pImage, pLens;      // Sampling probability on image sensor and lens

            CameraSample camSample = camera.sample(pixelX, pixelY, rands);
            const Ray ray = camSample.generateRay();

            return Color(helper::radiance(scene, params, ray, rands, 0) * (camera.sensitivity() / camSample.totalPdf()));
        }

    }   // anonymous namespace

    PathTracingRenderer::PathTracingRenderer(spica::Image* image)
        : _image(image)
    {
    }

    PathTracingRenderer::~PathTracingRenderer()
    {
    }

    void PathTracingRenderer::render(const Scene& scene, const Camera& camera, const RenderParameters& params) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        // Prepare random number generators
        RandomSampler* samplers = new RandomSampler[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {
            switch (params.randomType()) {
            case PSEUDO_RANDOM_TWISTER:
                printf("Use pseudo random numbers (Twister)\n");
                samplers[i] = Random::factory(i);
                break;

            case QUASI_MONTE_CARLO:
                printf("Use quasi random numbers (Halton)\n");
                samplers[i] = Halton::factory(200, true, i);
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
        for (int i = 0; i < params.samplePerPixel(); i++) {
            for (int t = 0; t < taskPerThread; t++) {
                ompfor (int threadID = 0; threadID < OMP_NUM_CORE; threadID++) {
                    if (t < tasks[threadID].size()) {
                        Stack<double> rstk;
                        const int y = tasks[threadID][t];
                        for (int x = 0; x < width; x++) {
                            samplers[threadID].request(&rstk, 200);
                            buffer.pixel(width - x - 1, y) += executePathTracing(scene, camera, params, x, y, rstk);
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

            printf("  %6.2f %%  processed -> %s\r", 100.0 * (i + 1) / params.samplePerPixel(), filename);
        }
        printf("\nFinish!!\n");

        delete[] samplers;

        if (isAllocImageInside) {
            delete _image;
        }
    }

}  // namespace spica
