#ifndef _SPICA_HALTON_H_
#define _SPICA_HALTON_H_

#include "random.h"

namespace spica {

    /** Randomized Halton sampler for quasi Monte Carlo.
     *  @ingroup random_module
     */
    class SPICA_EXPORTS Halton : public RandomBase {
    private:
        static const int nPrimes = 1000;
        int  dims;
        int* bases;
        int* permute;
        int  numUsedSamples;

    public:
        /** Constructor.
         *  @param[in] dim: Dimension of halton sequence.
         *  @param[in] isPermute: Whether or not use permuted Halton.
         *  @param[in] seed: If permuted Halton is used, it is a seed for random for the permutation.
         */
        explicit Halton(int dim = 200, bool isPermute = true, unsigned int seed = 0);
        ~Halton();

        /** Request specified amount of random numbers
         */
        void request(Stack<double>* rstk, const int numRequested) override;

        static RandomSampler factory(int dim = 200, bool isPermute = true, unsigned int seed = 0);

    private:
        double radicalInverse(int n, int base, const int* p) const;
    };

}

#endif
