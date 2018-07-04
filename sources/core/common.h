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
#   define SPICA_IMPORTS __attribute__((visibility ("hidden")))
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
static const double EPS = 1.0e-12;

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
#define Assertion(PREDICATE, ...) \
do { \
    if (!(PREDICATE)) { \
        std::cerr << "Asssertion \"" \
        << #PREDICATE << "\" failed in " << __FILE__ \
        << " line " << __LINE__ \
        << " in function \"" << (__FUNCTION_NAME__) << "\"" \
        << " : "; \
        fprintf(stderr, __VA_ARGS__); \
        std::cerr << std::endl; \
        std::abort(); \
    } \
} while (false)
#else  // NDEBUG
#define Assertion(PREDICATE, ...) do {} while (false)
#endif  // NDEBUG

// -----------------------------------------------------------------------------
// Message handlers
// -----------------------------------------------------------------------------

#ifndef NDEBUG
#define MsgInfo(...) \
do { \
    std::cout << "[INFO] "; \
    fprintf(stdout, __VA_ARGS__); \
    std::cerr << std::endl; \
} while (false);
#define Warning(...) \
do { \
    std::cerr << "[WARNING] "; \
    fprintf(stdout, __VA_ARGS__); \
    std::cerr << std::endl; \
} while (false);
#else
#define SpicaInfo(...)
#define SpicaWarn(...)
#endif
#define FatalError(...) \
do { \
    std::cerr << "[ERROR] "; \
    fprintf(stderr, __VA_ARGS__); \
    std::cerr << std::endl; \
    std::abort(); \
} while (false);

// -----------------------------------------------------------------------------
// Alignment
// -----------------------------------------------------------------------------

#if defined(_WIN32) || defined(__WIN32__)
inline void* align_alloc(size_t size, size_t alignsize) {
    return _aligned_malloc(size, alignsize);
}
inline void align_free(void* mem) { 
    if (!mem) return;
    _aligned_free(mem);
}
#else
inline void* align_alloc(size_t size, size_t alignsize) {
    void* mem = nullptr;
    int ret = posix_memalign((void**)&mem, alignsize, size);
    return (ret == 0) ? mem : nullptr;
}
inline void align_free(void* mem) {
    if (!mem) return;
    free(mem);
}
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

inline double lerp(double t, double v1, double v2) {
    return (1.0 - t) * v1 + t * v2;
}

#endif  // _SPICA_COMMON_H_
