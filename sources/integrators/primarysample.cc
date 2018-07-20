#include "primarysample.h"

#include "core/random.h"

namespace spica {

PSSSampler::PSSSampler(uint32_t seed, double pLarge, int streamCount)
    : streamCount_{ streamCount }
    , rand_{ seed }
    , pLarge_{ pLarge } {
}

double PSSSampler::get1D() {
    const int index = currentStreamIndex_ + streamCount_ * currentCoordIndex_;
    checkReady(index);
    currentCoordIndex_ += 1;
    return currentSamples_[index].value();
}

void PSSSampler::checkReady(int index) {
    if (index >= (int)currentSamples_.size()) {
        const int size = currentSamples_.size();
        currentSamples_.resize(index + 1);
        previousSamples_.resize(index + 1);
        for (int i = size; i < index + 1; i++) {
            currentSamples_[i] = PSSSample(0, rand_.get1D());
            previousSamples_[i] = PSSSample(0, rand_.get1D());
        }
    }

    if (currentSamples_[index].modifyTime() < globalTime_) {
        if (largeStep_) {
            // Large step
            previousSamples_[index] = currentSamples_[index];
            currentSamples_[index] = PSSSample(globalTime_, rand_.get1D());
        } else {
            // Small step
            if (currentSamples_[index].modifyTime() < largeStepTime_) {
                previousSamples_[index] = currentSamples_[index];
                currentSamples_[index] = PSSSample(largeStepTime_, rand_.get1D());
            }

            while (currentSamples_[index].modifyTime() < globalTime_ - 1) {
                currentSamples_[index].mutate(rand_.get2D());
            }
            previousSamples_[index] = currentSamples_[index];
            currentSamples_[index].mutate(rand_.get2D());
        }
    }
}

bool PSSSampler::startNextSample() {
    currentCoordIndex_ = 0;
    currentStreamIndex_ = 0;
    largeStep_ = rand_.get1D() < pLarge_;
    return true;
}

void PSSSampler::startStream(int streamIndex) {
    Assertion(streamIndex < streamCount_, "Stream index is out of bounds!");
    currentCoordIndex_ = 0;
    currentStreamIndex_ = streamIndex;
}

std::unique_ptr<Sampler> PSSSampler::clone(uint32_t seed) const {
    return std::make_unique<PSSSampler>(seed, pLarge_, streamCount_);
}

void PSSSampler::accept() {
    if (largeStep_) {
        largeStepTime_ = globalTime_;
    }
    globalTime_++;
}

void PSSSampler::reject() {
    for (int p = 0; p < (int)currentSamples_.size(); p++) {
        if (currentSamples_[p].modifyTime() == globalTime_) {
            currentSamples_[p] = previousSamples_[p];
        }
    }
}

}  // namespace spica