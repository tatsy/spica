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

#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_COMMON_H_
#define SPICA_COMMON_H_

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include <typeinfo>
#include <type_traits>

#if defined(_WIN32) || defined(__WIN32__)
#else
#include <cxxabi.h>
#endif

#include "spica_dirs.h"

// ----------------------------------------------------------------------------
// Parameter constants
// ----------------------------------------------------------------------------
static const double PI = 4.0 * atan(1.0);
static const double INV_PI = 1.0 / PI;
static const double INFTY = 1.0e128;
static const double EPS = 1.0e-6;

// ----------------------------------------------------------------------------
// Parallel for
// ----------------------------------------------------------------------------
#ifdef _OPENMP
    #include <omp.h>
    #if defined(_WIN32) || defined(__WIN32__)
        #define ompfor __pragma(omp parallel for) for
        #define omplock __pragma(omp critical)
    #else
        #define ompfor _Pragma("omp parallel for") for
        #define omplock _Pragma("omp critical")
    #endif
    const int kNumCores = omp_get_max_threads();
    inline int omp_thread_id() { return omp_get_thread_num(); }
#else  // _OPENMP
    #define ompfor for
    #define omplock
    const int kNumCores = 1;
    inline int omp_thread_id() { return 0; }
#endif  // _OPENMP

// ----------------------------------------------------------------------------
// Assertion with message
// ----------------------------------------------------------------------------
#undef NDEBUG
#ifndef NDEBUG
#define Assertion(PREDICATE, MSG) \
do { \
    if (!(PREDICATE)) { \
        std::cerr << "Asssertion \"" \
        << #PREDICATE << "\" failed in " << __FILE__ \
        << " line " << __LINE__ << " : " << MSG << std::endl; \
        std::abort(); \
    } \
} while (false)
#else  // NDEBUG
#define Assertion(PREDICATE, MSG) do {} while (false)
#endif  // NDEBUG

// ----------------------------------------------------------------------------
// Alignment
// ----------------------------------------------------------------------------

#if defined(_WIN32) || defined(__WIN32__)
    #define align_attrib(typ, siz) __declspec(align(siz)) typ
#else
    #define align_attrib(typ, siz) typ __attribute__((aligned(siz)))
#endif

// ----------------------------------------------------------------------------
// isnan / isinf
// ----------------------------------------------------------------------------
#if defined(_WIN32) || defined(__WIN32__)
    #if _MSC_VER <= 1600
        #define isnan(x) _isnan(x)
        #define isinf(x) (!_finite(x))
    #endif
#endif


// ----------------------------------------------------------------------------
// Utility functions
// ----------------------------------------------------------------------------

#define WITH_ENABLER
#if defined(_WIN32) || defined(__WIN32__)
    #if _MSC_VER <= 1600
        #undef WITH_ENABLER
    #endif
#endif

#ifdef WITH_ENABLER
extern void* enabler;
#endif

#ifdef WITH_ENABLER
template <class Ty, 
          typename
          std::enable_if<std::is_arithmetic<Ty>::value>::type *& = enabler>
#else
template <class Ty>
#endif
inline Ty clamp(Ty v, Ty lo, Ty hi) {
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    return v;
}

#ifdef WITH_ENABLER
template <class Ty, 
          typename 
          std::enable_if<std::is_arithmetic<Ty>::value>::type *& = enabler>
#else
template <class Ty>
#endif
inline Ty max3(Ty a, Ty b, Ty c) {
    return std::max(a, std::max(b, c));
}

#endif  // _SPICA_COMMON_H_
