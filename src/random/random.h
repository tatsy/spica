#ifndef _SPICA_RANDOM_H_
#define _SPICA_RANDOM_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_RANDOM_EXPORT
        #define SPICA_RANDOM_DLL __declspec(dllexport)
    #else
        #define SPICA_RANDOM_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_RANDOM_DLL
#endif

#include "../utils/common.h"
#include "random_base.h"

namespace spica {

    // --------------------------------------------------
    // Random number generator with Mersenne twister
    // --------------------------------------------------
    class SPICA_RANDOM_DLL Random : public RandomBase {
    public:
        explicit Random(int seed = -1);

        // Generate a random integer from [0, n-1]
        int nextInt(const int n) const;

        // Generate a floating point random number from [0, 1)
        double nextReal() const;

        // Request specified amount of random numbers
        void requestSamples(RandomSeq& rseq, const int numRequested);
    };

}

#endif  // _SPICA_RANDOM_H_
