#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RANDOM_SAMPLER_H_
#define _SPICA_RANDOM_SAMPLER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_RANDOM_SAMPLER_EXPORT
        #define SPICA_RANDOM_SAMPLER_DLL __declspec(dllexport)
    #else
        #define SPICA_RANDOM_SAMPLER_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_RANDOM_SAMPLER_DLL
#endif

#include <memory>

#include "random_base.h"
#include "../utils/stack.h"

namespace spica {

    class RandomSampler {
    private:
        std::unique_ptr<RandomBase> _rng;

    public:
        RandomSampler();
        ~RandomSampler();

        void request(Stack<double>* rands, int n);

        RandomSampler(RandomSampler&& rs);
        RandomSampler& operator=(RandomSampler&& rs);

    private:
        RandomSampler(const RandomSampler&) = delete;
        RandomSampler& operator=(const RandomSampler&) = delete;

        friend class Random;
        friend class Halton;
    };

}  // namespace spica

#endif  // _SPICA_RANDOM_SAMPLER_H_
