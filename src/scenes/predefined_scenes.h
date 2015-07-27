#ifndef _SPICA_PREDEFINED_SCENES_H_
#define _SPICA_PREDEFINED_SCENES_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_PREDEFINED_SCENES_EXPORT
        #define SPICA_PREDEFINED_SCENES_DLL __declspec(dllexport)
    #else
        #define SPICA_PREDEFINED_SCENES_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_PREDEFINED_SCENES_DLL
#endif

namespace spica {

    class Scene;
    class Camera;

    SPICA_PREDEFINED_SCENES_DLL void cornellBox(Scene* scene, Camera* camera, const int width, const int height);

    SPICA_PREDEFINED_SCENES_DLL void cornellBoxBunny(Scene* scene, Camera* camera, const int width, const int height);
   
    SPICA_PREDEFINED_SCENES_DLL void cornellBoxOcclusion(Scene* scene, Camera* camera, const int width, const int height);

    SPICA_PREDEFINED_SCENES_DLL void cornellBoxDragon(Scene* scene, Camera* camera, const int width, const int height);
    
    SPICA_PREDEFINED_SCENES_DLL void kittenBox(Scene* scene, Camera* camera, const int width, const int height);

    SPICA_PREDEFINED_SCENES_DLL void kittenEnvmap(Scene* scene, Camera* camera, const int width, const int height);

}

#endif  // _SPICA_CORNELL_BOX_H_
