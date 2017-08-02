#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_PARSER_
#define _SPICA_SCENE_PARSER_

#include <string>
#include <QtXml/qdom.h>

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
    void parseChildren(QDomNode &node);
    Transform parseTransform(QDomNode &node);
    std::shared_ptr<Primitive> createPrimitive(const std::shared_ptr<Shape> &shape,
                                               const Transform &transform,
                                               const std::shared_ptr<Material> &material);

    void storeToParam(QDomNode &node);

    std::string xmlFile_;
    PluginManager &plugins_;
    RenderParams &params_;

    std::shared_ptr<Camera> camera_ = nullptr;
    std::vector<std::shared_ptr<Primitive>> primitives_;
    std::vector<std::shared_ptr<Light>> lights_;
    bool waitAreaLight_ = false;
};

}  // namespace spica

#endif  // _SPICA_SCENE_PARSER_