#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_LOADER_H_
#define _SPICA_SCENE_LOADER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SCENE_LOADER_EXPORT
        #define SPICA_SCENE_LOADER_DLL __declspec(dllexport)
    #else
        #define SPICA_SCENE_LOADER_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_SCENE_LOADER_DLL
#endif

#include <string>

namespace spica {

    class Scene;
    class Camera;

    class SPICA_SCENE_LOADER_DLL SceneLoader {
    public:
        SceneLoader();
        ~SceneLoader();    

        void load(const std::string& filename,
                  Scene* scene, Camera* camera) const;
    };

}  // namespace spica

#endif  // _SPICA_SCENE_LOADER_H_
