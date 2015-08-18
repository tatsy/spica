#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RANDOM_BASE_H_
#define _SPICA_RANDOM_BASE_H_

#if defined(_WIN32) || defined(__WIN32__)
#define SPICA_RANDOM_BASE_DLL __declspec(dllexport)
#else
#define SPICA_RANDOM_BASE_DLL
#endif

#include <vector>

#include "../utils/uncopyable.h"
#include "../utils/stack.h"

namespace spica {

    // --------------------------------------------------
    // Interface class for random number generator
    // --------------------------------------------------
    class RandomBase {
    public:
        virtual ~RandomBase() {}

        // Request specified amount of random numbers
        virtual void request(Stack<double>* randSeq, const int numRequested) = 0;
    };
};

#endif  // _SPICA_RANDOM_BASE_H_
