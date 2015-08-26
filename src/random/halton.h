#ifndef _SPICA_HALTON_H_
#define _SPICA_HALTON_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_HALTON_EXPORT
        #define SPICA_HALTON_DLL __declspec(dllexport)
    #else
        #define SPICA_HALTON_DLL __declspec(dllexport)
    #endif
#else
    #define SPICA_HALTON_DLL
#endif

#include "random.h"

namespace spica {

    // --------------------------------------------------
    // Randomized Halton sampler for quasi Monte Carlo
    // --------------------------------------------------
    class SPICA_HALTON_DLL Halton : public RandomBase {
    private:
        static const int nPrimes = 1000;
        int  dims;
        int* bases;
        int* permute;
        int  numUsedSamples;

    public:
        // Constructor
        // @param[in] dim: dimension of halton sequence
        // @param[in] isPermute: whether or not use permuted Halton
        // @param[in] seed: if permuted Halton is used, it is a seed for random for the permutation
        explicit Halton(int dim = 200, bool isPermute = true, unsigned int seed = 0);
        ~Halton();

        // Request specified amount of random numbers
        void request(Stack<double>* rstk, const int numRequested) override;

        static RandomSampler factory(int dim = 200, bool isPermute = true, unsigned int seed = 0);

    private:
        double radicalInverse(int n, int base, const int* p) const;
    };

}

#endif
