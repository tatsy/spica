#define SPICA_SCENE_LOADER_EXPORT
#include "scene_loader.h"

#include "yaml_parser.h"
#include "../renderer/scene.h"
#include "../renderer/camera.h"

namespace spica {

    SceneLoader::SceneLoader() {
    }

    SceneLoader::~SceneLoader() {
    }

    void SceneLoader::load(const std::string& filename,
                           Scene* scene, Camera* camera) const {
        // Load yaml
        YamlParser parser(filename);
        YamlElement root = parser.documentElement();
    }

}  // namespace spica
