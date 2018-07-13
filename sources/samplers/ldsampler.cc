#define SPICA_API_EXPORT
#include "ldsampler.h"

#include "core/renderparams.h"

namespace spica {

namespace {

int countBits(int bits) {
    bits = (bits & 0x55555555) + (bits >> 1 & 0x55555555);
    bits = (bits & 0x33333333) + (bits >> 2 & 0x33333333);
    bits = (bits & 0x0f0f0f0f) + (bits >> 4 & 0x0f0f0f0f);
    bits = (bits & 0x00ff00ff) + (bits >> 8 & 0x00ff00ff);
    return (bits & 0x0000ffff) + (bits >>16 & 0x0000ffff);
}

int countTrailingZeros(int i) {
    return countBits(i & (-i));
}

template <class T>
void shuffle(T *startPtr, T *endPtr,
             int bucketSize, Random &rng) {
    const int nBucket = (endPtr - startPtr) / bucketSize;
    for (int i = 0; i < nBucket; i++) {
        int r = i + rng.nextInt(nBucket - i);
        for (int j = 0; j < bucketSize; j++) {
            const int a = i * bucketSize + j;
            const int b = r * bucketSize + j;
            std::swap(*(startPtr + a), *(startPtr + b));
        }
    }
}

void grayCodeSample(const uint32_t *C, int nSamples, uint32_t scramble, double *samples) {
    uint32_t v = scramble;
    for (int i = 0; i < nSamples; i++) {
        samples[i] = std::min(v * (0x1p-32), 1.0 - EPS);
        v ^= C[countTrailingZeros(i + 1)];
    }
}

void grayCodeSample(const uint32_t *C0, const uint32_t *C1,
                    int nSamples, const Point2i &scrambles, Point2d *samples) {
    uint32_t u = scrambles[0];
    uint32_t v = scrambles[1];

    for (int i = 0; i < nSamples; i++) {
        double sampleX = std::min(u * (0x1p-32), 1.0 - EPS);
        double sampleY = std::min(v * (0x1p-32), 1.0 - EPS);
        samples[i] = Point2d(sampleX, sampleY);
        u ^= C0[countTrailingZeros(i + 1)];
        v ^= C1[countTrailingZeros(i + 1)];
    }
}

void vanDerCorput(int nSamplesPerPixel, int nPixelSamples, double *samples, Random &rng) {
    static const uint32_t CVanDerCorput[32] = {
        0x80000000, 0x40000000, 0x20000000, 0x10000000,
        0x8000000,  0x4000000,  0x2000000,  0x1000000,
        0x800000,   0x400000,   0x200000,   0x100000,
        0x80000,    0x40000,    0x20000,    0x10000,
        0x8000,     0x4000,     0x2000,     0x1000,
        0x800,      0x400,      0x200,      0x100,
        0x80,       0x40,       0x20,       0x10,
        0x8,        0x4,        0x2,        0x1
    };

    uint32_t scramble = rng.nextInt();
    const int totalSamples = nSamplesPerPixel * nPixelSamples;
    grayCodeSample(CVanDerCorput, totalSamples, scramble, samples);
    for (int i = 0; i < nPixelSamples; i++) {
        shuffle(samples + i * nSamplesPerPixel,
                samples + (i + 1) * nSamplesPerPixel, 1, rng);
    }
    shuffle(samples, samples + totalSamples, nSamplesPerPixel, rng);
}

void sobol2D(int nSamplesPerPixel, int nPixelSamples, Point2d *samples, Random &rng) {
    static const uint32_t CSobol2D[2][32] = {
        {
            0x80000000, 0x40000000, 0x20000000, 0x10000000,
            0x8000000,  0x4000000,  0x2000000,  0x1000000,
            0x800000,   0x400000,   0x200000,   0x100000,
            0x80000,    0x40000,    0x20000,    0x10000,
            0x8000,     0x4000,     0x2000,     0x1000,
            0x800,      0x400,      0x200,      0x100,
            0x80,       0x40,       0x20,       0x10,
            0x8,        0x4,        0x2,        0x1
        }, {
            0x80000000, 0xc0000000, 0xa0000000, 0xf0000000,
            0x88000000, 0xcc000000, 0xaa000000, 0xff000000,
            0x80800000, 0xc0c00000, 0xa0a00000, 0xf0f00000,
            0x88880000, 0xcccc0000, 0xaaaa0000, 0xffff0000,
            0x80008000, 0xc000c000, 0xa000a000, 0xf000f000,
            0x88008800, 0xcc00cc00, 0xaa00aa00, 0xff00ff00,
            0x80808080, 0xc0c0c0c0, 0xa0a0a0a0, 0xf0f0f0f0,
            0x88888888, 0xcccccccc, 0xaaaaaaaa, 0xffffffff
        }
    };

    Point2i scramble = Point2i(rng.nextInt(), rng.nextInt());
    const int totalSamples = nSamplesPerPixel * nPixelSamples;
    grayCodeSample(CSobol2D[0], CSobol2D[1], totalSamples, scramble, samples);
    for (int i = 0; i < nPixelSamples; i++) {
        shuffle(samples + i * nSamplesPerPixel,
                samples + (i + 1) * nSamplesPerPixel, 1, rng);
    }
    shuffle(samples, samples + totalSamples, nSamplesPerPixel, rng);
}

}  // Anonymous namespace



LowDiscrepancySampler::LowDiscrepancySampler(int samplesPerPixel, int nSampledDimensions, uint32_t seed)
        : samplesPerPixel_{samplesPerPixel}
        , nSampledDimensions_{nSampledDimensions}
        , rng_{seed} {
    initializeSamples();
}

LowDiscrepancySampler::LowDiscrepancySampler(RenderParams &params)
        : LowDiscrepancySampler{params.getInt("sampleCount", 32),
                                params.getInt("dimensions", 2, true)} {
}

LowDiscrepancySampler::~LowDiscrepancySampler() {
}

double LowDiscrepancySampler::get1D() {
    if (currentSample1DDim_ < nSampledDimensions_) {
        return sample1D_[currentSample1DDim_++][currentSampleIndex_];
    } else {
        return rng_.get1D();
    }
}

Point2d LowDiscrepancySampler::get2D() {
    if (currentSample2DDim_ < nSampledDimensions_) {
        return sample2D_[currentSample2DDim_++][currentSampleIndex_];
    } else {
        return rng_.get2D();
    }
}

bool LowDiscrepancySampler::startNextSample() {
    currentSample1DDim_ = 0;
    currentSample2DDim_ = 0;
    return ++currentSampleIndex_ < samplesPerPixel_;
}

void LowDiscrepancySampler::startPixel() {
    currentSample1DDim_ = 0;
    currentSample2DDim_ = 0;
    currentSampleIndex_ = 0;
}

std::unique_ptr<Sampler> LowDiscrepancySampler::clone(uint32_t seed) const {
    return std::make_unique<LowDiscrepancySampler>(samplesPerPixel_, nSampledDimensions_, seed);
}

void LowDiscrepancySampler::initializeSamples() {
    // 1D samples
    sample1D_.assign(nSampledDimensions_,
                     std::vector<double>(samplesPerPixel_, 0.0));
    for (int i = 0; i < nSampledDimensions_; i++) {
        vanDerCorput(1, samplesPerPixel_, &sample1D_[i][0], rng_);
    }

    // 2D samples
    sample2D_.assign(nSampledDimensions_,
                     std::vector<Point2d>(samplesPerPixel_, Point2d(0.0, 0.0)));
    for (int i = 0; i < nSampledDimensions_; i++) {
        sobol2D(1, samplesPerPixel_, &sample2D_[i][0], rng_);
    }
}

}  // namespace spica