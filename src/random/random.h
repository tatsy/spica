#ifdef _MSC_VER
#pragma once
#endif

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

    class RandomSampler;

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
        explicit Random(unsigned int seed = 0);

        // Generate a random integer from [0, n-1]
        int nextInt();
        int nextInt(const int n);

        // Generate a floating point random number from [0, 1)
        double nextReal();

        // Request specified amount of random numbers
        void request(Stack<double>* rands, const int numRequested) override;

        // Factory method for RandomSampler
        static RandomSampler factory(unsigned int seed = 0);

    private:
        void init_genrand(unsigned int s);
        unsigned int genrand_int32(void);
        int genrand_int31(void);
        double genrand_real2(void);
    };

}

#endif  // _SPICA_RANDOM_H_
