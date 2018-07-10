#include "sceneparser.h"

#include <memory>
#include <stdexcept>

#include "core/cobject.h"
#include "core/parallel.h"
#include "core/camera.h"
#include "core/shape.h"
#include "core/material.h"
#include "core/medium.h"
#include "core/integrator.h"
#include "core/primitive.h"
#include "core/meshio.h"

using namespace tinyxml2;

namespace spica {

SceneParser::SceneParser()
    : params_{RenderParams::getInstance()}
    , plugins_{PluginManager::getInstance()}{
    // Initialize default rendering parameters
    params_.add("maxDepth", 16);
    params_.add("accelerator", std::string("bvh"));
}

SceneParser::SceneParser(const std::string &xmlFile)
    : SceneParser{} {
    this->xmlFile_ = xmlFile;
}

void SceneParser::parse() {
    XMLDocument doc;
    doc.LoadFile(xmlFile_.c_str());
    //QFile file(xmlFile_.c_str());
    //if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //    char errmsg[512];
    //    sprintf(errmsg, "Failed to open file: %s\n", xmlFile_.c_str());
    //    throw std::runtime_error(errmsg);
    //}

    //QDomDocument doc;
    //doc.setContent(&file);

    //QDomElement root = doc.documentElement();
    //Assertion(root.nodeName() == "scene",
    //          "XML root node should be \"scene\"!");
    //printf("Version: %s\n", root.attribute("version").toStdString().c_str());

    //parseChildren(root);
    //Assertion(camera_ != nullptr, "Sensor is not specified!");

    //const std::string integType = params_.getString("integrator");
    //plugins_.initModule(integType);
    //auto integrator = std::shared_ptr<Integrator>((Integrator*)plugins_.createObject(integType, params_));

    //const std::string accelType = params_.getString("accelerator");
    //plugins_.initAccelerator(accelType);

    //auto accelerator = std::shared_ptr<Accelerator>(plugins_.createAccelerator(accelType, primitives_, params_));
    //Scene scene(accelerator, lights_);

    //setNumThreads(params_.getInt("numUserThreads"));
    //printf("Threads: %d\n", numSystemThreads());

    //integrator->render(camera_, scene, params_);
}

void SceneParser::parseChildren(XMLNode &parent) {
    //QDomNode node = parent.firstChild();
    //while (!node.isNull()) {
    //    if (node.hasChildNodes() && node.nodeName() != "transform") {
    //        parseChildren(node);
    //    }
    //    storeToParam(node);

    //    node = node.nextSibling();
    //}
}

Transform SceneParser::parseTransform(XMLNode &parent) {
    //QDomNode node = parent.firstChild();

    //Transform trans;
    //while (!node.isNull()) {
    //    const auto &subAttrs = node.attributes();

    //    Transform sub;
    //    if (node.nodeName() == "matrix") {
    //        const QString values = subAttrs.namedItem("value").nodeValue();
    //        QStringList valueList = values.split(" ");
    //        Assertion(valueList.length() == 16, "# of matrix values is not 16!");

    //        double m[4][4];
    //        for (int i = 0; i < 4; i++) {
    //            for (int j = 0; j < 4; j++) {
    //                m[i][j] = valueList[i * 4 + j].toDouble();
    //            }
    //        }
    //        sub = Transform(m);
    //    } else if (node.nodeName() == "scale") {
    //        const double x = subAttrs.namedItem("x").nodeValue().toDouble();
    //        const double y = subAttrs.namedItem("y").nodeValue().toDouble();
    //        const double z = subAttrs.namedItem("z").nodeValue().toDouble();
    //        sub = Transform::scale(x, y, z);
    //    } else if (node.nodeName() == "rotate") {
    //        const double x = subAttrs.namedItem("x").nodeValue().toDouble();
    //        const double y = subAttrs.namedItem("y").nodeValue().toDouble();
    //        const double z = subAttrs.namedItem("z").nodeValue().toDouble();
    //        const double angle = subAttrs.namedItem("angle").nodeValue().toDouble();
    //        sub = Transform::rotate(angle, Vector3d(x, y, z));        
    //    } else if (node.nodeName() == "translate") {
    //        const double x = subAttrs.namedItem("x").nodeValue().toDouble();
    //        const double y = subAttrs.namedItem("y").nodeValue().toDouble();
    //        const double z = subAttrs.namedItem("z").nodeValue().toDouble();
    //        sub = Transform::translate(Vector3d(x, y, z));                
    //    } else if (node.nodeName() == "lookAt") {
    //        const Vector3d origin = Vector3d(subAttrs.namedItem("origin").nodeValue().toStdString());
    //        const Vector3d target = Vector3d(subAttrs.namedItem("target").nodeValue().toStdString());
    //        const Vector3d up = Vector3d(subAttrs.namedItem("up").nodeValue().toStdString());
    //        sub = Transform::lookAt(Point3d(origin), Point3d(target), up);
    //    }
    //    trans = sub * trans;

    //    node = node.nextSibling();
    //}

    //return std::move(trans);
    return Transform{};
}

std::shared_ptr<Primitive> SceneParser::createPrimitive(const std::shared_ptr<Shape> &shape,
                                                        const Transform &transform,
                                                        const std::shared_ptr<Material> &material) {
    //std::shared_ptr<Light> light = nullptr;
    //if (waitAreaLight_) {
    //    params_.add("shape", std::static_pointer_cast<CObject>(shape));
    //    params_.add("toWorld", transform);
    //    plugins_.initModule("area");
    //    light = std::shared_ptr<Light>((Light*)plugins_.createObject("area", params_));
    //    lights_.push_back(light);
    //}
    //
    //return std::make_shared<GeometricPrimitive>(shape, material, light);
    return std::make_shared<GeometricPrimitive>(nullptr, nullptr, nullptr);
}

void SceneParser::storeToParam(XMLNode &node) {
    //const std::string nodeName = node.nodeName().toStdString();
    //if (nodeName == "#comment") return;

    ////printf("Parsing: %s\n", nodeName.c_str());

    //const auto &attrs = node.attributes();

    //std::string name = "";
    //if (!attrs.namedItem("name").isNull()) {
    //    name = attrs.namedItem("name").nodeValue().toStdString();        
    //}

    //if (nodeName == "boolean") {
    //    bool value = attrs.namedItem("value").nodeValue() == "true";
    //    if (name != "") {
    //        params_.add(name, value);
    //    }
    //} else if (nodeName == "integer") {
    //    int value = attrs.namedItem("value").nodeValue().toInt();
    //    if (name != "") {
    //        params_.add(name, value);
    //    }
    //} else if (nodeName == "float") {
    //    double value = attrs.namedItem("value").nodeValue().toDouble();
    //    if (name != "") {
    //        params_.add(name, value);
    //    }
    //} else if (nodeName == "string") {
    //    std::string value = attrs.namedItem("value").nodeValue().toStdString();
    //    if (name == "filename") {
    //        QFileInfo fileinfo(xmlFile_.c_str());
    //        value = fileinfo.absoluteDir().filePath(value.c_str()).toStdString();            
    //    }

    //    if (name != "") {
    //        params_.add(name, value);
    //    }
    //} else if (nodeName == "rgb") {
    //    Vector3d v(attrs.namedItem("value").nodeValue().toStdString());
    //    Spectrum value(v.x(), v.y(), v.z());
    //    if (name != "") {
    //        params_.add(name, value);
    //    }
    //} else if (nodeName == "spectrum") {
    //    QString str = attrs.namedItem("value").nodeValue();
    //    QStringList items = str.split(",");
    //    std::vector<double> nm;
    //    std::vector<double> specs;
    //    Q_FOREACH(const QString &it, items) {
    //        QStringList temp = it.split(":");
    //        if (temp.length() >= 2) {
    //            nm.push_back(temp[0].toDouble());
    //            specs.push_back(temp[1].toDouble());
    //        }
    //    }
    //    Spectrum value(nm, specs);
    //    if (name != "") {
    //        params_.add(name, value);
    //    }
    //} else if (nodeName == "transform") {
    //    Transform value = parseTransform(node);
    //    if (name != "") {
    //        params_.add(name, value);
    //    }
    //} else if (nodeName == "shape") {
    //    std::string type = attrs.namedItem("type").nodeValue().toStdString();

    //    auto surface = std::static_pointer_cast<SurfaceMaterial>(params_.getObject("bsdf", nullptr, true));
    //    auto subsurface = std::static_pointer_cast<SubsurfaceMaterial>(params_.getObject("subsurface", nullptr, true));
    //    auto material = std::make_shared<Material>(surface, subsurface);
    //    auto transform = params_.getTransform("toWorld", Transform(), true);
    //    
    //    if (type == "obj") {
    //        QFileInfo fileinfo(xmlFile_.c_str());
    //        const std::string filename = params_.getString("filename");
    //        std::vector<ShapeGroup> groups = meshio::loadOBJ(filename, transform);
    //        for (const auto &g : groups) {
    //            for (const auto &s : g.shapes()) {
    //                primitives_.push_back(createPrimitive(s, transform, material));
    //            }                
    //        }
    //    } else if (type == "ply") {
    //        const std::string filename = params_.getString("filename");
    //        std::vector<ShapeGroup> groups = meshio::loadPLY(filename, transform);
    //        for (const auto &g : groups) {
    //            for (const auto &s : g.shapes()) {
    //                primitives_.push_back(createPrimitive(s, transform, material));
    //            }
    //        }
    //    } else {
    //        plugins_.initModule(type);
    //        auto value = std::shared_ptr<CObject>(plugins_.createObject(type, params_));
    //        auto s = std::static_pointer_cast<Shape>(value);
    //        primitives_.push_back(createPrimitive(s, transform, material));
    //    }

    //    waitAreaLight_ = false;

    //} else if (nodeName == "ref") {
    //    std::string id = attrs.namedItem("id").nodeValue().toStdString();
    //    auto object = params_.getObject(id);

    //    if (dynamic_cast<SurfaceMaterial*>(object.get())) {
    //        params_.add("bsdf", object);            
    //    } else if (dynamic_cast<SubsurfaceMaterial*>(object.get())) {
    //        params_.add("subsurface", object);
    //    } else if (dynamic_cast<Medium*>(object.get())) {
    //        params_.add("medium", object);        
    //    }
    //} else if (nodeName == "integrator") {
    //    std::string type = attrs.namedItem("type").nodeValue().toStdString();
    //    Assertion(type != "", "Integrator type is not specified!");
    //    params_.add("integrator", type);
    //} else {
    //    std::string type = attrs.namedItem("type").nodeValue().toStdString();
    //    if (nodeName == "emitter" && type == "area") {
    //        waitAreaLight_ = true;
    //        return;
    //    }

    //    Assertion(type != "", "Type parameter is not specified for \"%s\"", nodeName.c_str());

    //    plugins_.initModule(type);
    //    auto value = std::shared_ptr<CObject>(plugins_.createObject(type, params_));

    //    if (name != "") {
    //        params_.add(name, value);
    //    } else {
    //        std::string id = attrs.namedItem("id").nodeValue().toStdString();
    //        if (id != "") {
    //            params_.add(id, value);
    //        } else {
    //            params_.add(nodeName, value);
    //        }
    //    }

    //    if (nodeName == "sensor") {
    //        Assertion(!camera_, "Multiple cameras are specified!");
    //        camera_ = std::static_pointer_cast<Camera>(value);
    //    } else if (nodeName == "emitter") {
    //        lights_.push_back(std::static_pointer_cast<Light>(value));
    //    }
    //}
}

}  // namespace spica
