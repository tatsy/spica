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
        explicit Halton(int dim = 200, bool isPermute = true, Random& rng = Random());
        ~Halton();

        // Request specified amount of random numbers
        void requestSamples(RandomSeq& rseq, const int numRequested);

    private:
        double radicalInverse(int n, int base, const int* p) const;
    };

}

#endif
