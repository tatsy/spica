#define SPICA_API_EXPORT
#include "vbo.h"

#include <typeinfo>
#include <string>

#include "../shape/triangle.h"

namespace spica {

VBO::VBO()
    : vertices_()
    , normals_()
    , colors_()
    , indices_() {
}

VBO::VBO(const VBO& vbo)
    : vertices_()
    , normals_()
    , colors_()
    , indices_() {
    operator=(vbo);
}

VBO::~VBO() {
}

VBO& VBO::operator=(const VBO& vbo) {
    this->vertices_ = vbo.vertices_;
    this->normals_  = vbo.normals_;
    this->colors_   = vbo.colors_;
    this->indices_  = vbo.indices_;
    return *this;
}

void VBO::add(const Triangle& t, const RGBSpectrum& color) {
    const int triID = static_cast<int>(vertices_.size()) / 3;
    add(t[0], t.normal(0), color);
    add(t[1], t.normal(1), color);
    add(t[2], t.normal(2), color);

    indices_.push_back(triID + 0);
    indices_.push_back(triID + 1);
    indices_.push_back(triID + 2);
}

void VBO::add(const Point3d& v, const Normal3d& normal, const RGBSpectrum& color) {
    vertices_.push_back(v.x());
    vertices_.push_back(v.y());
    vertices_.push_back(v.z());
    normals_.push_back(normal.x());
    normals_.push_back(normal.y());
    normals_.push_back(normal.z());
    colors_.push_back(color.red());
    colors_.push_back(color.green());
    colors_.push_back(color.blue());
}

}  // namespace spica
