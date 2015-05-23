#ifndef _SPICA_HALTON_H_
#define _SPICA_HALTON_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_HALTON_EXPORT
        #define SPICA_HALTON_DLL __declspec(dllexport)
    #else
        #define SPICA_HALTON_DLL __declspec(dllexport)
    #endif
#else
    #define SPICA_HALTON_DLL
#endif


namespace spica {

    // --------------------------------------------------
    // Halton sampler for quasi Monte Carlo
    // --------------------------------------------------
    class SPICA_HALTON_DLL Halton {
    public:
        Halton();

        // Generate quasi random number
        // @param[in] baseID: index of prime number for base (ex, 0 -> 2, 1 -> 3, 2 -> 5, ...)
        // @param[in] seqID: index of random sequence
        double nextReal(const int baseID, const int seqID) const;
    };

}

#endif
