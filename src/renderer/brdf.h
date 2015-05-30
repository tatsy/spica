#ifndef _SPICA_BRDF_H_
#define _SPICA_BRDF_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_BRDF_EXPORT
        #define SPICA_BRDF_EXPORT __declspec(dllexport)
    #else
        #define SPICA_BRDF_EXPORT __declspec(dllimport)
    #endif
#else
    #define SPICA_BRDF_EXPORT
#endif

#include "../utils/vector3.h"

namespace spica {

    class BRDF {
    public:
        BRDF() {}
        virtual ~BRDF() {}
        virtual double evalPdf(const Vector3& in, const Vector3& normal, const Vector3& out) const = 0;
        virtual void sample(const Vector3& in, const Vector3& normal, Vector3* out) const = 0;
    };

}

#endif  // _SPICA_BRDF_H_
