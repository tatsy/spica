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

#ifndef _SPICA_COMMON_H_
#define _SPICA_COMMON_H_

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

// -----------------------------------------------------------------------------
// API export macro
// -----------------------------------------------------------------------------

#if (defined(WIN32) || defined(_WIN32) || defined(WINCE) || defined(__CYGWIN__))
#   if defined(SPICA_API_EXPORT)
#       define SPICA_EXPORTS __declspec(dllexport)
#       define SPICA_IMPORTS
#   else
#       define SPICA_EXPORTS
#       define SPICA_IMPORTS __declspec(dllimport)
#   endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define SPICA_EXPORTS __attribute__((visibility ("default")))
#   define SPICA_IMPORTS
#else
#   define SPICA_EXPORTS
#   define SPICA_IMPORTS
#endif

#if (defined(WIN32) || defined(_WIN32) || defined(WINCE) || defined(__CYGWIN__))
#   define PACKED(__declare__) __pragma(pack(push,1)) __declare__ __pragma(pack(pop)) 
#else
#   define PACKED(__declare__) __declare__ __attribute__((__packed__))
#endif

// ----------------------------------------------------------------------------
// Parameter constants
// ----------------------------------------------------------------------------
static const double PI = 4.0 * atan(1.0);
static const double INV_PI = 1.0 / PI;
static const double INFTY = 1.0e32;
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
    const int kNumThreads = omp_get_max_threads();
    inline int omp_thread_id() { return omp_get_thread_num(); }
#else  // _OPENMP
    #define ompfor for
    #define omplock
    const int kNumThreads = 1;
    inline int omp_thread_id() { return 0; }
#endif  // _OPENMP

// -----------------------------------------------------------------------------
// Assertion with message
// -----------------------------------------------------------------------------

#ifndef __FUNCTION_NAME__
    #if defined(_WIN32) || defined(__WIN32__)
        #define __FUNCTION_NAME__ __FUNCTION__
    #else
        #define __FUNCTION_NAME__ __func__
    #endif
#endif

#undef NDEBUG
#ifndef NDEBUG
#define Assertion(PREDICATE, MSG, ...) \
do { \
    if (!(PREDICATE)) { \
        std::cerr << "Asssertion \"" \
        << #PREDICATE << "\" failed in " << __FILE__ \
        << " line " << __LINE__ \
        << " in function \"" << (__FUNCTION_NAME__) << "\"" \
        << " : "; \
        fprintf(stderr, MSG, __VA_ARGS__); \
        std::cerr << std::endl; \
        std::abort(); \
    } \
} while (false)
#else  // NDEBUG
#define Assertion(PREDICATE, MSG, ...) do {} while (false)
#endif  // NDEBUG

// -----------------------------------------------------------------------------
// Message handlers
// -----------------------------------------------------------------------------

#ifndef NDEBUG
#define MsgInfo(MSG, ...) \
do { \
    std::cout << "[INFO] "; \
    fprintf(stderr, MSG, __VA_ARGS__); \
    std::cerr << std::endl; \
} while (false);
#define Warning(MSG, ...) \
do { \
    std::cerr << "[WARNING] " << (MSG) << std::endl; \
} while (false);
#else
#define SpicaInfo(MSG)
#define SpicaWarn(MSG)
#endif
#define FatalError(MSG) \
do { \
    std::cerr << "[ERROR] " << (MSG) << std::endl; \
    std::abort(); \
} while (false);

// -----------------------------------------------------------------------------
// Alignment
// -----------------------------------------------------------------------------

#if defined(_WIN32) || defined(__WIN32__)
inline void* align_alloc(size_t size, size_t alignsize) {
    return _aligned_malloc(size, alignsize);
}
inline void align_free(void* mem) { _aligned_free(mem); }
#else
inline void* align_alloc(size_t size, size_t alignsize) {
    void* mem = nullptr;
    int ret = posix_memalign((void**)&mem, alignsize, size);
    return (ret == 0) ? mem : nullptr;
}
inline void align_free(void* mem) { free(mem); }
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

extern void* enabler;

template <class T,
          typename std::enable_if<std::is_arithmetic<T>::value>::type *& = enabler>
inline T clamp(T v, T lo, T hi) {
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    return v;
}

template <class T,
          typename std::enable_if<std::is_arithmetic<T>::value>::type *& = enabler>
inline T max3(T a, T b, T c) {
    return std::max(a, std::max(b, c));
}

template <class T,
          typename std::enable_if<std::is_arithmetic<T>::value>::type *& = enabler>
inline T min3(T a, T b, T c) {
    return std::min(a, std::min(b, c));
}

#endif  // _SPICA_COMMON_H_
