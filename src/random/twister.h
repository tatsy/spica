/******************************************************************************
Copyright 2015 Tatsuya Yatagawa (tatsy)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#ifndef _SPICA_TWISTER_H_
#define _SPICA_TWISTER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_TWISTER_EXPORT
        #define SPICA_TWISTER_DLL __declspec(dllexport)
    #else
        #define SPICA_TWISTER_DLL __declspec(dllexport)
    #endif
#else
    #define SPICA_TWISTER_DLL
#endif

#include "random_base.h"

namespace spica {

    // --------------------------------------------------
    // Random number generator with Mersenne twister
    // --------------------------------------------------
    class SPICA_TWISTER_DLL Twister : public RandomBase {
    public:
        explicit Twister(int seed = -1);

    public:
        // Generate a random integer from [0, n-1]
        int nextInt(const int n) const;

        // Generate a floating point random number from [0, 1)
        double nextReal() const;

    };  // class Twister

}  // namespace spica

#endif  // _SPICA_TWISTER_H_
