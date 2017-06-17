#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RANDOM_H_
#define _SPICA_RANDOM_H_

#include "../core/common.h"
#include "sampler.h"

namespace spica {

    class Sampler;

    /**
     * Random number generator with Mersenne twister.
     * @ingroup random_module
     */
    class SPICA_EXPORTS Random : public Sampler {
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

        int nextInt();

        /** Generate a random integer from [0, n-1].
         */
        int nextInt(const int n);

        /** Generate a floating point random number from [0, 1).
         */
        double nextReal();

        /** Generate a random number from normal distribution.
         */
        double normal();

        double get1D() override;

        std::unique_ptr<Sampler> clone(unsigned int seed = 0) const override;

    private:
        void init_genrand(unsigned int s);
        unsigned int genrand_int32(void);
        int genrand_int31(void);
        double genrand_real2(void);
    };

}  // namespace spica

#endif  // _SPICA_RANDOM_H_
