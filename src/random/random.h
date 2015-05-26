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
    private:
        static const int N = 624;
        static const int M = 397;
        static const unsigned int MATRIX_A;
        static const unsigned int UPPER_MASK;
        static const unsigned int LOWER_MASK;

        unsigned int mt[N];
        int mti;

    public:
        explicit Random(int seed = -1);

        // Generate a random integer from [0, n-1]
        int nextInt(const int n);

        // Generate a floating point random number from [0, 1)
        double nextReal();

        // Request specified amount of random numbers
        void requestSamples(RandomSeq& rseq, const int numRequested);

    private:
        void init_genrand(unsigned int s);
        void init_by_array(unsigned int init_key[], int key_length);
        unsigned int genrand_int32(void);
        int genrand_int31(void);
        double genrand_real1(void);
        double genrand_real2(void);
        double genrand_real3(void);
        double genrand_res53(void);
    };

}

#endif  // _SPICA_RANDOM_H_
