#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_LOADER_H_
#define _SPICA_SCENE_LOADER_H_

#include <string>

#include "../core/common.h"

namespace spica {

    class Scene;
    class Camera;

    class SPICA_EXPORTS SceneLoader {
    public:
        SceneLoader();
        ~SceneLoader();    

        void load(const std::string& filename,
                  Scene* scene, Camera* camera) const;
    };

}  // namespace spica

#endif  // _SPICA_SCENE_LOADER_H_
