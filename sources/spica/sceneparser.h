#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_PARSER_
#define _SPICA_SCENE_PARSER_

#include <string>
#include <tinyxml2.h>

#include "core/cobject.h"
#include "core/renderparams.h"
#include "core/scene.h"

namespace spica {

class SceneParser {
public:
    SceneParser();
    SceneParser(const std::string &file);

    void parse();

private:
    void parseChildren(const tinyxml2::XMLElement *node);
    Transform parseTransform(const tinyxml2::XMLElement *node);

    std::shared_ptr<Primitive> createPrimitive(const std::shared_ptr<Shape> &shape,
                                               const Transform &transform,
                                               const std::shared_ptr<Material> &material,
                                               const std::shared_ptr<Medium> &medium);

    void storeToParam(const tinyxml2::XMLElement *node);

    std::string xmlFile_;
    PluginManager &plugins_;
    RenderParams &params_;

    std::shared_ptr<Camera> camera_ = nullptr;
    std::vector<std::shared_ptr<Primitive>> primitives_;
    std::vector<std::shared_ptr<Light>> lights_;
    std::vector<std::shared_ptr<Medium>> mediums_;
    bool waitAreaLight_ = false;
};

}

#endif  // _SPICA_SCENE_PARSER_