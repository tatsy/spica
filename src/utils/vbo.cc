#define SPICA_VBO_EXPORT
#include "vbo.h"

#include <typeinfo>
#include <string>

namespace spica {

    VBO::VBO()
        : _vertices()
        , _normals()
        , _colors()
        , _indices()
    {
    }

    VBO::VBO(const VBO& vbo)
        : _vertices()
        , _normals()
        , _colors()
        , _indices()
    {
        operator=(vbo);
    }

    VBO::~VBO()
    {
    }

    VBO& VBO::operator=(const VBO& vbo) {
        this->_vertices = vbo._vertices;
        this->_normals = vbo._normals;
        this->_colors = vbo._colors;
        this->_indices = vbo._indices;
        return *this;
    }

    void VBO::add(const Primitive* p, const Color& color) {
        std::string typname = get_typename(p);
        std::cout << typname << std::endl;
        if (typname == "class spica::Quad") {
            const Quad* quad = reinterpret_cast<const Quad*>(p);
            add(*quad, color);
        } else if (typname == "class spica::Sphere") {
            const Sphere* sphere = reinterpret_cast<const Sphere*>(p);
            add(*sphere, color);
        } else if (typname == "class spica::Trimesh") {
            const Trimesh* trimesh = reinterpret_cast<const Trimesh*>(p);
            add(*trimesh, color);
        } else {
            msg_assert(false, (typname + "is not supported").c_str());        
        }
    }

    void VBO::add(const Vector3& v, const Vector3& normal, const Color& color) {
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

    void VBO::add(const Quad& quad, const Color& color) {
        unsigned int idx = static_cast<unsigned int>(_vertices.size()) / 3;
        add(quad.p0(), quad.normal(), color);            
        add(quad.p1(), quad.normal(), color);            
        add(quad.p2(), quad.normal(), color);            
        add(quad.p3(), quad.normal(), color);
        _indices.push_back(idx);
        _indices.push_back(idx + 1);
        _indices.push_back(idx + 2);
        _indices.push_back(idx + 2);
        _indices.push_back(idx + 3);
        _indices.push_back(idx + 0);
    }

    void VBO::add(const Triangle& tri, const Color& color) {
        unsigned int idx = static_cast<unsigned int>(_vertices.size()) / 3;
        add(tri.p0(), tri.normal(), color);
        add(tri.p1(), tri.normal(), color);
        add(tri.p2(), tri.normal(), color);
        _indices.push_back(idx);
        _indices.push_back(idx + 1);
        _indices.push_back(idx + 2);
    }

    void VBO::add(const Sphere& sphere, const Color& color) {
        static const int nTheta = 32;
        static const int nPhi   = 64;

        for (int i = 0; i < nTheta; i++) {
            for (int j = 0; j < nPhi; j++) {
                double theta0 = PI * i / nTheta;
                double theta1 = PI * (i + 1) / nTheta; 
                double phi0 = 2.0 * PI * j / nPhi;
                double phi1 = 2.0 * PI * (j + 1) / nPhi;

                double st0 = sin(theta0);
                double st1 = sin(theta1);
                double ct0 = cos(theta0);
                double ct1 = cos(theta1);
                double sp0 = sin(phi0);
                double sp1 = sin(phi1);
                double cp0 = cos(phi0);
                double cp1 = cos(phi1);

                Vector3 v00 = sphere.center() + sphere.radius() * Vector3(cp0 * st0, sp0 * st0, ct0);
                Vector3 v01 = sphere.center() + sphere.radius() * Vector3(cp0 * st1, sp0 * st1, ct1);
                Vector3 v10 = sphere.center() + sphere.radius() * Vector3(cp1 * st0, sp1 * st0, ct0);
                Vector3 v11 = sphere.center() + sphere.radius() * Vector3(cp1 * st1, sp1 * st1, ct1);

                unsigned int idx = static_cast<unsigned int>(_vertices.size()) / 3;
                if (i == 0) {
                    add(v00, (v00 - sphere.center()).normalized(), color);
                    add(v01, (v01 - sphere.center()).normalized(), color);
                    add(v11, (v11 - sphere.center()).normalized(), color);
                    _indices.push_back(idx);
                    _indices.push_back(idx + 1);
                    _indices.push_back(idx + 2);
                } else if (i == nTheta - 1) {
                    add(v00, (v00 - sphere.center()).normalized(), color);
                    add(v10, (v10 - sphere.center()).normalized(), color);
                    add(v01, (v01 - sphere.center()).normalized(), color);
                    _indices.push_back(idx);
                    _indices.push_back(idx + 1);
                    _indices.push_back(idx + 2);
                } else {
                    add(v00, (v00 - sphere.center()).normalized(), color);            
                    add(v01, (v01 - sphere.center()).normalized(), color);
                    add(v11, (v11 - sphere.center()).normalized(), color);
                    add(v10, (v10 - sphere.center()).normalized(), color);
                    _indices.push_back(idx);
                    _indices.push_back(idx + 1);
                    _indices.push_back(idx + 2);
                    _indices.push_back(idx + 2);
                    _indices.push_back(idx + 3);
                    _indices.push_back(idx + 0);
                }
            }
        }
    }

    void VBO::add(const Trimesh& trimesh, const Color& color) {
        for (int i = 0; i < trimesh.numFaces(); i++) {
            Triangle tri = trimesh.getTriangle(i);
            add(tri, color);
        }
    }


}  // namespace spica
