#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_LDSAMPLER_H
#define SPICA_LDSAMPLER_H

#include <vector>

#include "core/cobject.h"
#include "core/sampler.h"
#include "core/random.h"
#include "core/point2d.h"

namespace spica {

/** Low-discrepancy sampling with (0,2)-sequence.
 *  @ingroup random_module
 */
class SPICA_EXPORTS LowDiscrepancySampler : public Sampler {
public:
    LowDiscrepancySampler(int samplesPerPixel, int nSampledDimensions, uint32_t seed = 0);
    explicit LowDiscrepancySampler(RenderParams &params);
    ~LowDiscrepancySampler();

    double get1D() override;
    Point2d get2D() override;

    bool startNextSample() override;
    void startPixel() override;
    std::unique_ptr<Sampler> clone(uint32_t seed = 0) const override;

private:
    void initializeSamples();

    int samplesPerPixel_;
    int nSampledDimensions_;
    int currentSampleIndex_;
    int currentSample1DDim_;
    int currentSample2DDim_;
    std::vector<std::vector<double>> sample1D_;
    std::vector<std::vector<Point2d>> sample2D_;
    Random rng_;
};

SPICA_EXPORT_PLUGIN(LowDiscrepancySampler, "Low-discrepancy sampler with (0,2)-sequence.");

}  // namespace spica

#endif  // SPICA_LDSAMPLER_H
