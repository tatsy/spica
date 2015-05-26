#ifndef _SPICA_SUBSURFACE_SPPM_RENDERER_H_
#define _SPICA_SUBSURFACE_SPPM_RENDERER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SUBSURFACE_SPPM_EXPORT
        #define SPICA_SUBSURFACE_SPPM_DLL __declspec(dllexport)
    #else
        #define SPICA_SUBSURFACE_SPPM_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_SUBSURFACE_SPPM_DLL
#endif

#include "renderer_constants.h"

namespace spica {

    class SPICA_SUBSURFACE_SPPM_DLL SubsurfaceSPPMRenderer : public Uncopyable {
    public:
        SubsurfaceSPPMRenderer();
        ~SubsurfaceSPPMRenderer();
        
        struct HitpointInfo {
            
        };

    };

}

#endif  // _SPICA_SUBSURFACE_SPPM_RENDERER_H_
