#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RANDOM_BASE_H_
#define _SPICA_RANDOM_BASE_H_

#include <vector>

#include "../core/uncopyable.h"
#include "../core/stack.h"

namespace spica {

    /** Interface class for random number generator.
     *  @ingroup random_module
     */
    class SPICA_EXPORTS RandomBase {
    public:
        virtual ~RandomBase() {}

        /** Request specified amount of random numbers.
         */
        virtual void request(Stack<double>* rstack, const int numRequested) = 0;
    };

}  // namespace spica

#endif  // _SPICA_RANDOM_BASE_H_
