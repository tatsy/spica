#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_HALTON_H_
#define _SPICA_HALTON_H_

#include "random.h"

namespace spica {

    /** Randomized Halton sampler for quasi Monte Carlo.
     *  @ingroup random_module
     */
    class SPICA_EXPORTS Halton : public RandomInterface {
    public:
        /** Constructor.
         *  @param[in] dim: Dimension of halton sequence.
         *  @param[in] isPermute: Whether or not use permuted Halton.
         *  @param[in] seed: If permuted Halton is used, it is a seed for random for the permutation.
         */
        explicit Halton(int dim = 200, bool isPermute = true, unsigned int seed = 0);
        ~Halton();

        double get1D() override;

        std::unique_ptr<Sampler> clone(unsigned int seed = 0) const;

    private:
        double radicalInverse(int n, int base, const int* p) const;

        static const int nPrimes = 1000;
        int  dims;
        int* bases;
        int* permute;
        int  numUsedSamples;

    };

}  // namespace spica

#endif  // _SPICA_HALTON_H_
