#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_HALTON_H_
#define _SPICA_HALTON_H_

#include <memory>
#include "sampler.h"
#include "random.h"

namespace spica {

/** Randomized Halton sampler for quasi Monte Carlo.
 *  @ingroup random_module
 */
class SPICA_EXPORTS Halton : public Sampler {
public:
    /** Constructor.
     *  @param[in] ns: Number of samples used for one pixel.
     *  @param[in] isPermute: Whether or not use permuted Halton.
     *  @param[in] seed: If permuted Halton is used, it is a seed for random for the permutation.
     */
    explicit Halton(int ns = 200, bool isPermute = true,
                    unsigned int seed = 0);
    ~Halton();

    double get1D() override;

    bool startNextSample() override;

    std::unique_ptr<Sampler> clone(unsigned int seed = 0) const override;

private:
    double radicalInverse(int n, int base, const int* p) const;

    static const int nPrimes_ = 1000;
    int nSamples_;
    int sampleIndex_;
    int nUsedSamples_;
    bool isPermute_;
    Random rng_;
    const std::unique_ptr<int[]> bases_;
    const std::unique_ptr<int[]> permute_;
    const std::unique_ptr<double[]> samples_;

};

}  // namespace spica

#endif  // _SPICA_HALTON_H_
