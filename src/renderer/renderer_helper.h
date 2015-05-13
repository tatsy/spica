#ifndef _SPICA_RENDERER_HELPER_H_
#define _SPICA_RENDERER_HELPER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_RENDERER_HELPER_EXPORT
        #define SPICA_RENDERER_HELPER_DLL __declspec(dllexport)
    #else
        #define SPICA_RENDERER_HELPER_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_RENDERER_HELPER_DLL
#endif

#include "material.h"
#include "../utils/vector3.h"

namespace spica {

    namespace helper {

        bool SPICA_RENDERER_HELPER_DLL isTotalRef(const bool isIncoming,
                                                  const Vector3& position,
                                                  const Vector3& in,
                                                  const Vector3& normal,
                                                  const Vector3& orientNormal,
                                                  Vector3* reflectDir,
                                                  Vector3* refractDir,
                                                  double* fresnelRef,
                                                  double* fresnelTransmit);
    
    }

}

#endif  // _SPICA_RENDERER_HELPER_H_