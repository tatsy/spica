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
#include "../utils/uncopyable.h"

namespace spica {

    // --------------------------------------------------
    // Randomized Halton sampler for quasi Monte Carlo
    // --------------------------------------------------
    class SPICA_HALTON_DLL Halton : public Uncopyable {
    private:
        int  dims;
        int* bases;
        int* permute;

    public:
        Halton();
        ~Halton();

        Halton(int dim, const Random& rng);

        // Generate quasi random number
        // @param[in] baseID: index of prime number for base (ex, 0 -> 2, 1 -> 3, 2 -> 5, ...)
        // @param[in] seqID: index of random sequence
        double nextReal(const int baseID, const int seqID) const;

    private:
        double radicalInverse(int n, int base, const int* p) const;
    };

}

#endif
