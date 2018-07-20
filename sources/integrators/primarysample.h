#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PRIMARY_SAMPLE_H_
#define _SPICA_PRIMARY_SAMPLE_H_

#include "core/common.h"
#include "core/point2d.h"
#include "core/spectrum.h"
#include "core/sampler.h"
#include "core/random.h"

namespace spica {

/**
 * The sample object for primary space sampling.
 */
class PSSSample {
public:
    // Public methods
    PSSSample()
            : modifyTime_{0}, value_{0.0} {
    }

    PSSSample(int modifyTime, double value)
            : modifyTime_{modifyTime}, value_{value} {
    }

    PSSSample(const PSSSample &) = default;

    PSSSample &operator=(const PSSSample &) = default;

    void mutate(const Point2d &rand) {
        static const double s1 = 1.0 / 1024.0;
        static const double s2 = 1.0 / 64.0;

        const double dv = s2 * std::exp(-std::log(s2 / s1) * rand[0]);
        if (rand[1] < 0.5) {
            value_ += dv;
        } else {
            value_ -= dv;
        }

        value_ -= std::floor(value_);
        modifyTime_++;
    }

    inline int modifyTime() const { return modifyTime_; }

    inline double value() const { return value_; }

private:
    // Private fields
    int modifyTime_;
    double value_;
};

/**
 * Path sample.
 */
class PathSample {
public:
    PathSample()
        : p_{ 0.0, 0.0 }
        , L_{ 0.0, 0.0, 0.0 } {
    }

    PathSample(const Point2d& p, const Spectrum& L)
        : p_{ p }
        , L_{ L } {
    }

    PathSample(const PathSample&) = default;
    PathSample& operator=(const PathSample&) = default;

    inline const Point2d &p() const { return p_; }
    inline const Spectrum &L() const { return L_; }

private:
    Point2d p_;
    Spectrum L_;
};

/**
 * Management class for PSS sampling.
 */
class PSSSampler : public Sampler {
public:
    // Public methods
    PSSSampler(uint32_t seed, double pLarge, int streamCount = 1);

    double get1D() override;
    bool startNextSample() override;
    void startStream(int streamIndex);

    std::unique_ptr<Sampler> clone(uint32_t seed = 0) const override;

    void accept();
    void reject();

    inline int largeStep() const { return largeStep_ ? 1 : 0; }
    inline double pLarge() const { return pLarge_; }

private:
    // Private methods
    void checkReady(int index);

    // Private fields
    bool largeStep_ = false;
    int globalTime_ = 0;
    int largeStepTime_ = 0;
    int currentCoordIndex_ = 0;
    int currentStreamIndex_ = 0;
    int streamCount_ = 1;

    Random rand_;
    double pLarge_;

    std::vector<PSSSample> currentSamples_;
    std::vector<PSSSample> previousSamples_;
};

}  // namespace spica

#endif  // _SPICA_PRIMARY_SAMPLE_H_
