#ifndef _SPICA_RANDOM_BASE_H_
#define _SPICA_RANDOM_BASE_H_

#if defined(_WIN32) || defined(__WIN32__)
#define SPICA_RANDOM_BASE_DLL __declspec(dllexport)
#else
#define SPICA_RANDOM_BASE_DLL
#endif

namespace spica {
    
    // --------------------------------------------------
    // Interface class for random number generator
    // --------------------------------------------------
    class SPICA_RANDOM_BASE_DLL RandomBase {
    public:
        RandomBase() {}
        virtual ~RandomBase() {}
        virtual double nextReal() const = 0;
        virtual int nextInt(const int n) const = 0;
    };
};

#endif  // _SPICA_RANDOM_BASE_H_
