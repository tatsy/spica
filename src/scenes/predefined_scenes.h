#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PREDEFINED_SCENES_H_
#define _SPICA_PREDEFINED_SCENES_H_

#include "../core/common.h"

namespace spica {

    class Scene;
    class Camera;

    SPICA_EXPORTS void cornellBox(Scene* scene, Camera* camera, const int width, const int height);

    SPICA_EXPORTS void cornellBoxBunny(Scene* scene, Camera* camera, const int width, const int height);
   
    SPICA_EXPORTS void cornellBoxOcclusion(Scene* scene, Camera* camera, const int width, const int height);

    SPICA_EXPORTS void cornellBoxDragon(Scene* scene, Camera* camera, const int width, const int height);
    
    SPICA_EXPORTS void kittenBox(Scene* scene, Camera* camera, const int width, const int height);

    SPICA_EXPORTS void kittenEnvmap(Scene* scene, Camera* camera, const int width, const int height);

}

#endif  // _SPICA_CORNELL_BOX_H_
