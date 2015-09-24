#define SPICA_LIGHTING_EXPORT
#include "lighting.h"

#include "envmap.h"
#include "area_light.h"

#include "../renderer/scene.h"

namespace spica {

    Lighting::Lighting()
        : _ptr(nullptr) {
    }

    Lighting::Lighting(const Lighting& l)
        : _ptr(nullptr) {
        this->operator=(l);
    }

    Lighting::Lighting(Lighting&& l)
        : _ptr(nullptr) {
        this->operator=(std::move(l));
    }

    Lighting::~Lighting() {
        delete _ptr;
    }

    Lighting& Lighting::operator=(const Lighting& l) {
        this->_ptr = l._ptr->clone();
        return *this;
    }

    Lighting& Lighting::operator=(Lighting&& l) {
        this->_ptr = l._ptr;
        l._ptr = nullptr;
        return *this;
    }

    Lighting Lighting::asAreaLight(const std::vector<Triangle>& triangles,
                                   const Color& emittance) {
        Lighting l;
        l._ptr = new AreaLight(triangles, emittance);
        return std::move(l);
    }

    Lighting Lighting::asEnvmap(const Sphere& boundSphere, const std::string& filename) {
        Lighting l;
        l._ptr = new Envmap(boundSphere, filename);
        return std::move(l);
    }

    LightSample Lighting::sample(Stack<double>& rstack) const {
        Assertion(_ptr != nullptr, "Light pointer is null!!");
        return _ptr->sample(rstack.pop(), rstack.pop(), rstack.pop());
    }

    Color Lighting::directLight(const Vector3D& dir) const {
        Assertion(_ptr != nullptr, "Light pointer is null!!");
        return _ptr->directLight(dir);
    }

    double Lighting::area() const {
        Assertion(_ptr != nullptr, "Light pointer is null!!");
        return _ptr->area();
    }

}  // namespace spica