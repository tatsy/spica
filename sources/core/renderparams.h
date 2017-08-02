#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDER_PARAMETERS_H_
#define _SPICA_RENDER_PARAMETERS_H_

#include <string>
#include <memory>
#include <unordered_map>

#include "common.h"
#include "point2d.h"
#include "vector2d.h"
#include "bounds2d.h"
#include "point3d.h"
#include "vector3d.h"
#include "bounds3d.h"
#include "normal3d.h"
#include "spectrum.h"
#include "transform.h"

namespace spica {

class CObject;

class SPICA_EXPORTS RenderParams {
public:
    static RenderParams &getInstance();

    ~RenderParams();

    RenderParams(RenderParams &&params);
    RenderParams &operator=(RenderParams &&params);
    
    void clear();
    
    template <class T>
    void add(const std::string &name, const T &value);

    bool getBool(const std::string &name, bool remove = false);
    bool getBool(const std::string &name, bool value, bool remove = false);
    int getInt(const std::string &name, bool remove = false);
    int getInt(const std::string &name, int value, bool remove = false);
    double getDouble(const std::string &name, bool remove = false);
    double getDouble(const std::string &name, double value, bool remove = false);
    std::string getString(const std::string &name, bool remove = false);
    std::string getString(const std::string &name, const std::string &value, bool remove = false);
    Point2d getPoint2d(const std::string &name, bool remove = false);
    Vector2d getVector2d(const std::string &name, bool remove = false);
    Bounds2d getBounds2d(const std::string &name, bool remove = false);
    Point3d getPoint3d(const std::string &name, bool remove = false);
    Vector3d getVector3d(const std::string &name, bool remove = false);
    Bounds3d getBounds3d(const std::string &name, bool remove = false);
    Normal3d getNormal3d(const std::string &name, bool remove = false);
    Spectrum getSpectrum(const std::string &name, bool remove = false);
    Transform getTransform(const std::string &name, bool remove = false);
    Transform getTransform(const std::string &name, const Transform &value, bool remove = false);
    std::shared_ptr<CObject> getTexture(const std::string &name, bool remove = false);
    std::shared_ptr<CObject> getObject(const std::string &name, bool remove = false);
    std::shared_ptr<CObject> getObject(const std::string &name, const std::shared_ptr<CObject> &value, bool remove = false);

private:
    RenderParams();
    RenderParams(RenderParams &) = delete;
    RenderParams &operator=(RenderParams &) = delete;
    
private:
    // Private fields
    std::unordered_map<std::string, bool> bools;
    std::unordered_map<std::string, int> ints;
    std::unordered_map<std::string, double> doubles;
    std::unordered_map<std::string, Point2d> point2ds;
    std::unordered_map<std::string, Vector2d> vector2ds;
    std::unordered_map<std::string, Bounds2d> bounds2ds;
    std::unordered_map<std::string, Point3d> point3ds;
    std::unordered_map<std::string, Vector3d> vector3ds;
    std::unordered_map<std::string, Bounds3d> bounds3ds;
    std::unordered_map<std::string, Normal3d> normals;
    std::unordered_map<std::string, Spectrum> spectrums;
    std::unordered_map<std::string, Transform> transforms;
    std::unordered_map<std::string, std::string> strings;
    std::unordered_map<std::string, std::shared_ptr<CObject>> objects;
};

}  // namespace spica

#endif  // _SPICA_RENDER_PARAMETERS_H_
