/**
 *  @defgroup random_module Random
 *  @brief Random number generators.
 */
#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RANDOM_SAMPLER_H_
#define _SPICA_RANDOM_SAMPLER_H_

#include <memory>

#include "random_base.h"
#include "../core/stack.h"

namespace spica {

    /** Random sampler class.
     *  @ingroup random_module
     */
    class SPICA_EXPORTS RandomSampler : private Uncopyable {
    private:
        std::unique_ptr<RandomBase> _rng;

    public:
        RandomSampler();
        ~RandomSampler();

        void request(Stack<double>* rands, int n);

        RandomSampler(RandomSampler&& rs);
        RandomSampler& operator=(RandomSampler&& rs);

        /** Initialize random sampler using Mersenne Twister.
         *  @param seed: Seed number for the random number generator.
         */
        static RandomSampler useMersenne(unsigned int seed = 0U);

        /** Initialize random sampler using Halton sequence.
         *  @param dim: # of prime numbers for Halton sequence.
         *  @param isPermute: Permute the primes or not.
         *  @param seed: Seed number for random permutation.
         */
        static RandomSampler useHalton(int dim, bool isPermute,
                                       unsigned int seed = 0U);

        // Friend classes.
        friend class Random;
        friend class Halton;
    };

}  // namespace spica

#endif  // _SPICA_RANDOM_SAMPLER_H_
