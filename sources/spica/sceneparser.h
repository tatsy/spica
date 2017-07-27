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

    void storeToParam(QDomNode &node);

    std::string xmlFile_;
    PluginManager &plugins_;
    RenderParams &params_;

    std::shared_ptr<Integrator> integrator_ = nullptr;
    std::vector<std::shared_ptr<Primitive>> primitives_;
    std::vector<std::shared_ptr<Light>> lights_;
};

}  // namespace spica

#endif  // _SPICA_SCENE_PARSER_