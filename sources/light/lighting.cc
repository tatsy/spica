#define SPICA_API_EXPORT
#include "lighting.h"

#include "envmap.h"
#include "area_light.h"

#include "../scenes/scene.h"
#include "../renderer//photon_map.h"

namespace spica {

    Lighting::Lighting()
        : _ptr() {
    }

    Lighting::Lighting(const Lighting& l)
        : _ptr() {
        this->operator=(l);
    }

    Lighting::Lighting(Lighting&& l)
        : _ptr() {
        this->operator=(std::move(l));
    }

    Lighting::~Lighting() {
    }

    Lighting& Lighting::operator=(const Lighting& l) {
        this->_ptr = std::unique_ptr<Light>(l._ptr->clone());
        return *this;
    }

    Lighting& Lighting::operator=(Lighting&& l) {
        this->_ptr = std::move(l._ptr);
        l._ptr = nullptr;
        return *this;
    }

    Lighting Lighting::asAreaLight(const Trimesh& triangles,
                                   const Spectrum& emittance) {
        Lighting l;
        l._ptr = std::make_unique<AreaLight>(triangles, emittance);
        return std::move(l);
    }

    Lighting Lighting::asEnvmap(const Sphere& boundSphere, const std::string& filename) {
        Lighting l;
        l._ptr = std::make_unique<Envmap>(boundSphere, filename);
        return std::move(l);
    }

    Lighting Lighting::asEnvmap(const Sphere& boundSphere, const Image& image) {
        Lighting l;
        l._ptr = std::make_unique<Envmap>(boundSphere, image);
        return std::move(l);
    }

    LightSample Lighting::sample(const Vector3D& v, Stack<double>& rands) const {
        Assertion(_ptr != nullptr, "Light pointer is null!!");
        return _ptr->sample(v, rands);
    }

    Photon Lighting::samplePhoton(Stack<double>& rands) const {
        Assertion(_ptr != nullptr, "Light pointer is null!!");
        return _ptr->samplePhoton(rands);        
    }

    Spectrum Lighting::directLight(const Vector3D& dir) const {
        Assertion(_ptr != nullptr, "Light pointer is null!!");
        return _ptr->directLight(dir);
    }

    Spectrum Lighting::globalLight(const Vector3D& dir) const {
        Assertion(_ptr != nullptr, "Light pointer is null!!");
        return _ptr->globalLight(dir);
    }

    Light* Lighting::ptr() const {
        return _ptr.get();
    }

    double Lighting::area() const {
        Assertion(_ptr != nullptr, "Light pointer is null!!");
        return _ptr->area();
    }

    LightType Lighting::type() const {
        return  _ptr->type();
    }

}  // namespace spica