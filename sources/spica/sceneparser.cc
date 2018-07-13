#include "sceneparser.h"

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <experimental/filesystem>

#include "core/cobject.h"
#include "core/parallel.h"
#include "core/camera.h"
#include "core/shape.h"
#include "core/material.h"
#include "core/medium.h"
#include "core/integrator.h"
#include "core/primitive.h"
#include "core/meshio.h"
#include "core/transform.h"

using namespace tinyxml2;
namespace fs = std::experimental::filesystem;

namespace {

std::vector<std::string> split(const std::string &str, const std::string &delim) {
    int prev = 0;
    int cur = 0;
    std::vector<std::string> ret;
    while ((cur = str.find_first_of(delim.c_str(), prev)) != std::string::npos) {
        ret.push_back(str.substr(prev, cur));
        prev = cur + delim.size();
    }
    ret.push_back(str.substr(prev));
    return std::move(ret);
}

double str2double(const std::string &str) {
	char *ep;
	double ret = strtod(str.c_str(), &ep);
	if (strlen(ep) != 0) {
		Warning("Following part could not be parsed as double: %s\n", ep);
	}
	return ret;
}

const char* getAttribute(const XMLElement *elem, const char *name) {
    const char *value = elem->Attribute(name);
    if (!value) {
        Warning("Element \"%s\" does not have attribute \"%s\"", elem->Name(), name);
    }
    return value;
}

}  // Anonymous namespace


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
    this->xmlFile_ = fs::canonical(fs::absolute(fs::path(xmlFile.c_str()))).string();
}

void SceneParser::parse() {
    XMLError error;
    XMLDocument doc;
    error = doc.LoadFile(xmlFile_.c_str());
    if (error != XML_SUCCESS) {
        FatalError("Failed to open file:%s\n", xmlFile_.c_str());
    }

    const XMLElement *root = doc.RootElement();
    Assertion(std::strcmp(root->Name(), "scene") == 0, "XML root node should be \"scene\"!");
    printf("Version: %s\n", root->Attribute("version"));

    parseChildren(root);
    Assertion(camera_ != nullptr, "Sensor is not specified!");

    const std::string integType = params_.getString("integrator");
    plugins_.initModule(integType);
    auto integrator = std::shared_ptr<Integrator>((Integrator*)plugins_.createObject(integType, params_));

    const std::string accelType = params_.getString("accelerator");
    plugins_.initAccelerator(accelType);

    auto accelerator = std::shared_ptr<Accelerator>(plugins_.createAccelerator(accelType, primitives_, params_));
    Scene scene(accelerator, lights_);

    // #thread
    setNumThreads(params_.getInt("numUserThreads"));

    // Print info
    MsgInfo("   Scene: %s", xmlFile_.c_str());
    MsgInfo("#threads: %d", numSystemThreads());

    // Rendering
    integrator->render(camera_, scene, params_);
}

void SceneParser::parseChildren(const XMLElement *parent) {
    const XMLElement *elem = parent->FirstChildElement();
    while (elem) {
        if (!elem->NoChildren() && strcmp(elem->Name(), "transform") != 0) {
            parseChildren(elem);
        }
        storeToParam(elem);

        elem = elem->NextSiblingElement();
    }
}

Transform SceneParser::parseTransform(const XMLElement *parent) {
    const XMLElement *elem = parent->FirstChildElement();

    Transform trans;
    while (elem) {
        Transform sub;
        if (std::strcmp(elem->Name(), "matrix") == 0) {
            const std::string values = std::string(getAttribute(elem, "value"));
            auto valueList = split(values, " ");
            Assertion(valueList.size() == 16, "# of matrix values is not 16!");

            double m[4][4];
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    m[i][j] = str2double(valueList[i * 4 + j]);
                }
            }
            sub = Transform(m);
        } else if (std::strcmp(elem->Name(), "scale") == 0) {
            const double x = str2double(getAttribute(elem, "x"));
            const double y = str2double(getAttribute(elem, "y"));
            const double z = str2double(getAttribute(elem, "z"));
            sub = Transform::scale(x, y, z);
        } else if (std::strcmp(elem->Name(), "rotate") == 0) {
            const double x = str2double(getAttribute(elem, "x"));
            const double y = str2double(getAttribute(elem, "y"));
            const double z = str2double(getAttribute(elem, "z"));
            const double angle = str2double(getAttribute(elem, "angle"));
            sub = Transform::rotate(angle, Vector3d(x, y, z));
        } else if (std::strcmp(elem->Name(), "translate") == 0) {
            const double x = str2double(getAttribute(elem, "x"));
            const double y = str2double(getAttribute(elem, "y"));
            const double z = str2double(getAttribute(elem, "z"));
            sub = Transform::translate(Vector3d(x, y, z));
        } else if (std::strcmp(elem->Name(), "lookAt") == 0) {
            const Vector3d origin = Vector3d(getAttribute(elem, "origin"));
            const Vector3d target = Vector3d(getAttribute(elem, "target"));
            const Vector3d up = Vector3d(getAttribute(elem, "up"));
            sub = Transform::lookAt(Point3d(origin), Point3d(target), up);
        }
        trans = sub * trans;

        elem = elem->NextSiblingElement();
    }

    return std::move(trans);
}

std::shared_ptr<Primitive> SceneParser::createPrimitive(const std::shared_ptr<Shape> &shape,
                                                        const Transform &transform,
                                                        const std::shared_ptr<Material> &material) {
    std::shared_ptr<Light> light = nullptr;
    if (waitAreaLight_) {
        params_.add("shape", std::static_pointer_cast<CObject>(shape));
        params_.add("toWorld", transform);
        plugins_.initModule("area");
        light = std::shared_ptr<Light>((Light*)plugins_.createObject("area", params_));
        lights_.push_back(light);
    }

    return std::make_shared<GeometricPrimitive>(shape, material, light);
}

void SceneParser::storeToParam(const XMLElement *elem) {
    const std::string nodeName = elem->Name();
    if (nodeName == "#comment") return;

    std::string name = "";
    if (elem->Attribute("name")) {
        name = elem->Attribute("name");
    }

    if (name == "boolean") {
        bool value = std::strcmp(elem->Attribute("value"), "true") == 0;
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "integer") {
        int value = elem->IntAttribute("value");
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "float") {
        double value = elem->DoubleAttribute("value");
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "string") {
        std::string value = elem->Attribute("value");
        if (name == "filename") {
            fs::path xmlPath(xmlFile_.c_str());
            fs::path filePath(value.c_str());
            value = fs::absolute(fs::canonical(xmlPath.parent_path() / filePath)).string();
        }

        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "rgb") {
        Vector3d v(elem->Attribute("value"));
        Spectrum value(v.x(), v.y(), v.z());
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "spectrum") {
        std::string str = elem->Attribute("value");
        auto items = split(str, ",");
        std::vector<double> nm;
        std::vector<double> specs;
        for (const auto &it : items) {
            auto temp = split(it, ":");
            if (temp.size() >= 2) {
                nm.push_back(str2double(temp[0]));
                specs.push_back(str2double(temp[1]));
            }
        }
        Spectrum value(nm, specs);
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "transform") {
        Transform value = parseTransform(elem);
        if (name != "") {
            params_.add(name, value);
        }
    } else if (nodeName == "shape") {
        std::string type = elem->Attribute("type");

        auto surface = std::static_pointer_cast<SurfaceMaterial>(params_.getObject("bsdf", nullptr, true));
        auto subsurface = std::static_pointer_cast<SubsurfaceMaterial>(params_.getObject("subsurface", nullptr, true));
        auto material = std::make_shared<Material>(surface, subsurface);
        auto transform = params_.getTransform("toWorld", Transform(), true);

        if (type == "obj") {
            const std::string filename = params_.getString("filename");
            std::vector<ShapeGroup> groups = meshio::loadOBJ(filename, transform);
            for (const auto &g : groups) {
                for (const auto &s : g.shapes()) {
                    primitives_.push_back(createPrimitive(s, transform, material));
                }
            }
        } else if (type == "ply") {
            const std::string filename = params_.getString("filename");
            std::vector<ShapeGroup> groups = meshio::loadPLY(filename, transform);
            for (const auto &g : groups) {
                for (const auto &s : g.shapes()) {
                    primitives_.push_back(createPrimitive(s, transform, material));
                }
            }
        } else {
            plugins_.initModule(type);
            auto value = std::shared_ptr<CObject>(plugins_.createObject(type, params_));
            auto s = std::static_pointer_cast<Shape>(value);
            primitives_.push_back(createPrimitive(s, transform, material));
        }

        waitAreaLight_ = false;

    } else if (nodeName == "ref") {
        std::string id = elem->Attribute("id");
        auto object = params_.getObject(id);

        if (dynamic_cast<SurfaceMaterial*>(object.get())) {
            params_.add("bsdf", object);
        } else if (dynamic_cast<SubsurfaceMaterial*>(object.get())) {
            params_.add("subsurface", object);
        } else if (dynamic_cast<Medium*>(object.get())) {
            params_.add("medium", object);
        }
    } else if (nodeName == "integrator") {
        std::string type = elem->Attribute("type");
        Assertion(type != "", "Integrator type is not specified!");
        params_.add("integrator", type);
    } else {
        std::string type = elem->Attribute("type");
        if (nodeName == "emitter" && type == "area") {
            waitAreaLight_ = true;
            return;
        }

        Assertion(type != "", "Type parameter is not specified for \"%s\"", nodeName.c_str());

        plugins_.initModule(type);
        auto value = std::shared_ptr<CObject>(plugins_.createObject(type, params_));

        if (name != "") {
            params_.add(name, value);
        } else {
            std::string id = "";
            if (elem->Attribute("id")) {
                id = elem->Attribute("id");
            }

            if (id != "") {
                params_.add(id, value);
            } else {
                params_.add(nodeName, value);
            }
        }

        if (nodeName == "sensor") {
            Assertion(!camera_, "Multiple cameras are specified!");
            camera_ = std::static_pointer_cast<Camera>(value);
        } else if (nodeName == "emitter") {
            lights_.push_back(std::static_pointer_cast<Light>(value));
        }
    }
}

}  // namespace spica