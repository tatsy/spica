#define SPICA_API_EXPORT
#include "renderparams.h"

#include "core/constant.h"

namespace spica {

RenderParams &RenderParams::getInstance() {
    static RenderParams instance;
    return instance;
}

RenderParams::RenderParams() {
}

RenderParams::~RenderParams() {
}

RenderParams::RenderParams(RenderParams &&params)
    : RenderParams() {
    this->operator=(std::move(params));
}

RenderParams &RenderParams::operator=(RenderParams &&params) {
    this->bools = std::move(params.bools);
    this->ints = std::move(params.ints);
    this->doubles = std::move(params.doubles);
    this->point2ds = std::move(params.point2ds);
    this->vector2ds = std::move(params.vector2ds);
    this->point3ds = std::move(params.point3ds);
    this->normals = std::move(params.normals);
    this->spectrums = std::move(params.spectrums);
    this->strings = std::move(params.strings);
    this->objects = std::move(params.objects);

    return *this;
}

void RenderParams::clear() {
    bools.clear();
    ints.clear();
    doubles.clear();
    point2ds.clear();
    vector2ds.clear();
    point3ds.clear();
    vector3ds.clear();
    normals.clear();
    spectrums.clear();
    strings.clear();
    objects.clear();
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const bool &value) {
    bools[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const int &value) {
    ints[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const double &value) {
    doubles[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const Point2d &value) {
    point2ds[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const Vector2d &value) {
    vector2ds[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const Bounds2d &value) {
    bounds2ds[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const Point3d &value) {
    point3ds[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const Vector3d &value) {
    vector3ds[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const Bounds3d &value) {
    bounds3ds[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const Normal3d &value) {
    normals[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const Spectrum &value) {
    spectrums[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const Transform &value) {
    transforms[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const std::string &value) {
    strings[name] = value;
}

template <>
void SPICA_EXPORTS RenderParams::add(const std::string &name, const std::shared_ptr<CObject> &value) {
    objects[name] = value;
}
 
bool RenderParams::getBool(const std::string &name, bool remove) {
    const auto it = bools.find(name);
    Assertion(it != bools.cend(), "Bool not found: name = %s", name.c_str());

    if (remove) {
        bool ret = it->second;
        bools.erase(it);
        return ret;
    }
    return it->second;
}

bool RenderParams::getBool(const std::string &name, bool value, bool remove) {
    const auto it = bools.find(name);
    if (it != bools.cend()) {
        if (remove) {
            const bool ret = it->second;
            bools.erase(it);
            return ret;
        }
    }

    return value;
}

int RenderParams::getInt(const std::string &name, bool remove) {
    const auto it = ints.find(name);
    Assertion(it != ints.cend(), "Int not found: name = %s", name.c_str());

    if (remove) {
        const int ret = it->second;
        ints.erase(it);
        return ret;
    }
    return it->second;
}

int RenderParams::getInt(const std::string &name, int value, bool remove) {
    const auto it = ints.find(name);
    if (it != ints.cend()) {
        if (remove) {
            const int ret = it->second;
            ints.erase(it);
            return ret;
        }
        return it->second;
    }

    return value;
}

double RenderParams::getDouble(const std::string &name, bool remove) {
    const auto it = doubles.find(name);
    Assertion(it != doubles.cend(), "Double not found: name = %s", name.c_str());

    if (remove) {
        const double ret = it->second;
        doubles.erase(it);
        return ret;
    }
    return it->second;
}

double RenderParams::getDouble(const std::string &name, double value, bool remove) {
    const auto it = doubles.find(name);
    if (it != doubles.cend()) {
        if (remove) {
            const double ret = it->second;
            doubles.erase(it);
            return ret;
        }
        return it->second;
    }

    return value;
}

std::string RenderParams::getString(const std::string &name, bool remove) {
    const auto it = strings.find(name);
    Assertion(it != strings.cend(), "String not found: name = %s", name.c_str());

    if (remove) {
        const std::string ret = it->second;
        strings.erase(it);
        return ret;
    }
    return it->second;
}

std::string RenderParams::getString(const std::string &name, const std::string &value, bool remove) {
    const auto it = strings.find(name);
    if (it != strings.cend()) {
        if (remove) {
            const std::string ret = it->second;
            strings.erase(it);
            return ret;
        }
        return it->second;
    }

    return value;
}

Point2d RenderParams::getPoint2d(const std::string &name, bool remove) {
    const auto it = point2ds.find(name);
    Assertion(it != point2ds.cend(), "Point2d not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        point2ds.erase(it);
        return ret;
    }
    return it->second;
}

Vector2d RenderParams::getVector2d(const std::string &name, bool remove) {
    const auto it = vector2ds.find(name);
    Assertion(it != vector2ds.cend(), "Vector2d not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        vector2ds.erase(it);
        return ret;
    }
    return it->second;
}

Bounds2d RenderParams::getBounds2d(const std::string &name, bool remove) {
    const auto it = bounds2ds.find(name);
    Assertion(it != bounds2ds.cend(), "Bounds2d not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        bounds2ds.erase(it);
        return ret;
    }
    return it->second;
}

Point3d RenderParams::getPoint3d(const std::string &name, bool remove) {
    const auto it = point3ds.find(name);
    Assertion(it != point3ds.cend(), "Point3d not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        point3ds.erase(it);
        return ret;
    }
    return it->second;
}

Point3d RenderParams::getPoint3d(const std::string &name, const Point3d &value, bool remove) {
    const auto it = point3ds.find(name);
    if (it != point3ds.cend()) {
        if (remove) {
            const auto ret = it->second;
            point3ds.erase(it);
            return ret;
        }
        return it->second;
    }
    return value;
}

Vector3d RenderParams::getVector3d(const std::string &name, bool remove) {
    const auto it = vector3ds.find(name);
    Assertion(it != vector3ds.cend(), "Vector3d not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        vector3ds.erase(it);
        return ret;
    }
    return it->second;
}

Bounds3d RenderParams::getBounds3d(const std::string &name, bool remove) {
    const auto it = bounds3ds.find(name);
    Assertion(it != bounds3ds.cend(), "Bounds3d not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        bounds3ds.erase(it);
        return ret;
    }
    return it->second;
}

Normal3d RenderParams::getNormal3d(const std::string &name, bool remove) {
    const auto it = normals.find(name);
    Assertion(it != normals.cend(), "Normal not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        normals.erase(it);
        return ret;
    }
    return it->second;
}

Spectrum RenderParams::getSpectrum(const std::string &name, bool remove) {
    const auto it = spectrums.find(name);
    Assertion(it != spectrums.cend(), "Spectrum not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        spectrums.erase(it);
        return ret;
    }
    return it->second;
}

Transform RenderParams::getTransform(const std::string &name, bool remove) {
    const auto it = transforms.find(name);
    Assertion(it != transforms.cend(), "Transform not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        transforms.erase(it);
        return ret;
    }
    return it->second;
}

Transform RenderParams::getTransform(const std::string &name, const Transform &value, bool remove) {
    const auto it = transforms.find(name);
    if (it != transforms.cend()) {
        if (remove) {
            const auto ret = it->second;
            transforms.erase(it);
            return ret;
        }
        return it->second;
    }

    return value;
}

std::shared_ptr<CObject> RenderParams::getTexture(const std::string &name, bool remove) {
    const auto it = objects.find(name);
    if (it != objects.cend()) {
        if (remove) {
            const auto ret = it->second;
            objects.erase(it);
            return ret;
        }
        return it->second;
    }

    const auto sit = spectrums.find(name);
    if (sit != spectrums.cend()) {
        Spectrum s = sit->second;
        if (remove) {
            spectrums.erase(sit);
        }
        return std::static_pointer_cast<CObject>(std::make_shared<ConstantTexture<Spectrum>>(s));
    }

    const auto dit = doubles.find(name);
    if (dit != doubles.cend()) {
        double d = dit->second;
        if (remove) {
            doubles.erase(dit);
        }
        return std::static_pointer_cast<CObject>(std::make_shared<ConstantTexture<double>>(d));
    }

    //Warning("Texture not found: name = \"%s\"", name.c_str());
    return nullptr;
}

std::shared_ptr<CObject> RenderParams::getObject(const std::string &name, bool remove) {
    const auto it = objects.find(name);
    Assertion(it != objects.cend(), "Object not found: name = %s", name.c_str());

    if (remove) {
        const auto ret = it->second;
        objects.erase(it);
        return ret;
    }
    return it->second;
}

std::shared_ptr<CObject> RenderParams::getObject(const std::string &name, const std::shared_ptr<CObject> &value, bool remove) {
    const auto it = objects.find(name);
    if (it != objects.cend()) {
        if (remove) {
            const auto ret = it->second;
            objects.erase(it);
            return ret;
        }
        return it->second;
    }

    return value;
}

}  // namespace spica
