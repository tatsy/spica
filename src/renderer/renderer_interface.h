#ifdef _MSC_VER
#pragma once
#endif

#ifndef _RENDERER_INTERFACE_H_
#define _RENDERER_INTERFACE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #define SPICA_RENDERER_INTERFACE_DLL __declspec(dllexport)
#else
    #define SPICA_RENDERER_INTERFACE_DLL
#endif

#include "../utils/uncopyable.h"
#include "../utils/image.h"

#include "scene.h"
#include "camera.h"
#include "render_parameters.h"

namespace spica {

    class SPICA_RENDERER_INTERFACE_DLL IRenderer : private Uncopyable {
        
    protected:
        Image _result;

    public:
        IRenderer() {}
        virtual ~IRenderer() {}

        virtual void render(const Scene& scene, const Camera& camera, 
                            const RenderParameters& params) = 0;
    };

}  // namespace spica

#endif  // _RENDERER_INTERFACE_H_
