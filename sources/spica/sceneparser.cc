#include "sceneparser.h"

#include <memory>
#include <stdexcept>

#include <QtCore/qfile.h>
#include <QtXml/qdom.h>

#include "core/cobject.h"

#include "core/shape.h"
#include "core/material.h"
#include "core/integrator.h"
#include "core/primitive.h"

namespace spica {

SceneParser::SceneParser()
    : params_{RenderParams::getInstance()}
    , plugins_{PluginManager::getInstance()}{
}

SceneParser::SceneParser(const std::string &xmlFile)
    : SceneParser{} {
    this->xmlFile_ = xmlFile;
}

void SceneParser::parse() {
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::shared_ptr<Accelerator> accelerator;
    std::vector<std::shared_ptr<Light>> lights;

    QFile file(xmlFile_.c_str());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        char errmsg[512];
        sprintf(errmsg, "Failed to open file: %s\n", xmlFile_.c_str());
        throw std::runtime_error(errmsg);
    }

    QDomDocument doc;
    doc.setContent(&file);

    QDomElement root = doc.documentElement();
    Assertion(root.nodeName() == "scene",
              "XML root node should be \"scene\"!");
    printf("Version: %s\n", root.attribute("version").toStdString().c_str());

    parseChildren(root);

    //Scene scene(accelerator, lights);
    //return std::move(scene);
}

void SceneParser::parseChildren(QDomNode &parent) {
    QDomNode node = parent.firstChild();
    while (!node.isNull()) {
        if (node.hasChildNodes() && node.nodeName() != "transform") {
            parseChildren(node);
        }
        storeToParam(node);

        node = node.nextSibling();
    }
}

Transform SceneParser::parseTransform(QDomNode &parent) {
    QDomNode node = parent.firstChild();

    Transform trans;
    while (!node.isNull()) {
        const auto &subAttrs = node.attributes();

        Transform sub;
        if (node.nodeName() == "scale") {
            const double x = subAttrs.namedItem("x").nodeValue().toDouble();
            const double y = subAttrs.namedItem("y").nodeValue().toDouble();
            const double z = subAttrs.namedItem("z").nodeValue().toDouble();
            sub = Transform::scale(x, y, z);        
        } else if (node.nodeName() == "rotate") {
            const double x = subAttrs.namedItem("x").nodeValue().toDouble();
            const double y = subAttrs.namedItem("y").nodeValue().toDouble();
            const double z = subAttrs.namedItem("z").nodeValue().toDouble();
            const double angle = subAttrs.namedItem("angle").nodeValue().toDouble();
            sub = Transform::rotate(angle, Vector3d(x, y, z));        
        } else if (node.nodeName() == "translate") {
            const double x = subAttrs.namedItem("x").nodeValue().toDouble();
            const double y = subAttrs.namedItem("y").nodeValue().toDouble();
            const double z = subAttrs.namedItem("z").nodeValue().toDouble();
            sub = Transform::translate(Vector3d(x, y, z));                
        } else if (node.nodeName() == "lookAt") {
            const Vector3d origin = Vector3d(subAttrs.namedItem("origin").nodeValue().toStdString());
            const Vector3d target = Vector3d(subAttrs.namedItem("target").nodeValue().toStdString());
            const Vector3d up = Vector3d(subAttrs.namedItem("up").nodeValue().toStdString());
            sub = Transform::lookAt(Point3d(origin), Point3d(target), up);
        }
        trans = trans * sub;

        node = node.nextSibling();
    }

    return std::move(trans);
}

void SceneParser::storeToParam(QDomNode &node) {
    const std::string nodeName = node.nodeName().toStdString();
    printf("Parsing: %s\n", nodeName.c_str());

    const auto &attrs = node.attributes();

    std::string name = "";
    if (!attrs.namedItem("name").isNull()) {
        name = attrs.namedItem("name").nodeValue().toStdString();        
    }

    if (nodeName == "boolean") {
        bool value = attrs.namedItem("value").nodeValue() == "true";
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "integer") {
        int value = attrs.namedItem("value").nodeValue().toInt();
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "float") {
        double value = attrs.namedItem("value").nodeValue().toDouble();
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "string") {
        std::string value = attrs.namedItem("value").nodeValue().toStdString();
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "rgb") {
        Vector3d v(attrs.namedItem("value").nodeValue().toStdString());
        Spectrum value(v.x(), v.y(), v.z());
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "transform") {
        Transform value = parseTransform(node);
        if (name != "") {
            params_.add(name, value);
        }
    } else {
        std::string type = attrs.namedItem("type").nodeValue().toStdString();
        if (nodeName == "integrator") return;
        plugins_.initModule(type);
        auto value = std::shared_ptr<CObject>(plugins_.createObject(type, params_));

        if (name != "") {
            params_.add(name, value);
        } else {
            params_.add(nodeName, value);
        }

        if (nodeName == "integrator") {
            //Assertion(!integrator_, "Multiple integrator is specified!");
            //integrator_ = std::static_pointer_cast<Integrator>(value);            
        } else if (nodeName == "shape") {
            auto shape = std::static_pointer_cast<Shape>(value);
            auto material = std::static_pointer_cast<Material>(params_.getObject("bsdf"));
            auto light = std::static_pointer_cast<Light>(params_.getObject("emitter"));
            auto primitive = std::make_shared<GeometricPrimitive>(shape, material, light);
            primitives_.push_back(std::static_pointer_cast<Primitive>(primitive));
        } else if (nodeName == "emitter") {
            lights_.push_back(std::static_pointer_cast<Light>(value));
        }
    }
}

}  // namespace spica