#ifndef SPICA_CAMERA_H_
#define SPICA_CAMERA_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_CAMERA_EXPORT
        #define SPICA_CAMERA_DLL __declspec(dllexport)
    #else
        #define SPICA_CAMERA_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_CAMERA_DLL
#endif

namespace spica {

    class Camera {
        
    };

}

#endif  // SPICA_CAMERA_H_
