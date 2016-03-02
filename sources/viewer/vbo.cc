#define SPICA_API_EXPORT
#include "vbo.h"

#include <typeinfo>
#include <string>

namespace spica {

VBO::VBO()
    : _vertices()
    , _normals()
    , _colors()
    , _indices() {
}

VBO::VBO(const VBO& vbo)
    : _vertices()
    , _normals()
    , _colors()
    , _indices() {
    operator=(vbo);
}

VBO::~VBO() {
}

VBO& VBO::operator=(const VBO& vbo) {
    this->_vertices = vbo._vertices;
    this->_normals  = vbo._normals;
    this->_colors   = vbo._colors;
    this->_indices  = vbo._indices;
    return *this;
}

void VBO::add(const Point3d& v, const Normal3d& normal, const RGBSpectrum& color) {
    _vertices.push_back(v.x());
    _vertices.push_back(v.y());
    _vertices.push_back(v.z());
    _normals.push_back(normal.x());
    _normals.push_back(normal.y());
    _normals.push_back(normal.z());
    _colors.push_back(color.red());
    _colors.push_back(color.green());
    _colors.push_back(color.blue());
}

}  // namespace spica
