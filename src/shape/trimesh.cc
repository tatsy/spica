#define SPICA_API_EXPORT
#include "trimesh.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "../core/common.h"
#include "../core/path.h"
#include "../core/triplet.h"
#include "../core/image.h"

#include "../math/quaternion.h"

#include "../scenes/vertex_data.h"
#include "../accel/accel.h"

#include "../renderer/ray.h"

#include "shape_interface.h"
#include "bbox.h"
#include "triangle.h"
#include "plane.h"
#include "meshio.h"

namespace spica {

    Trimesh::Trimesh()
        : IShape{ShapeType::Triangle}
        , _vertices{}
        , _faces{}
        , _accel{}
        , _texture{} {
    }

    Trimesh::Trimesh(const std::string& filename)
        : Trimesh{} {
        load(filename);
    }

    Trimesh::Trimesh(const std::vector<Vector3D>& vertices,
                     const std::vector<Triplet>& faceIDs) 
        : Trimesh{} {

        // Copy faces
        _faces.assign(faceIDs.cbegin(), faceIDs.cend());

        // Set vertex positions
        _vertices.resize(vertices.size());
        const int nverts = static_cast<int>(vertices.size());
        for (int i = 0; i < nverts; i++) {
            _vertices[i] = VertexData(vertices[i]);
        }

        // Compute vertex normals
        calcVertexNormals();
    }

    Trimesh::Trimesh(const std::vector<Triangle>& tris)
        : Trimesh{} {
        for (int i = 0; i < tris.size(); i++) {
            int idx = _vertices.size();
            for (int k = 0; k < 3; k++) {
                _vertices.emplace_back(tris[i][k], Color(0.0, 0.0, 0.0), tris[i].normal());
            }
            _faces.emplace_back(idx, idx + 1, idx + 2);
        }    
    }

    Trimesh::Trimesh(const std::vector<VertexData>& vertices,
                     const std::vector<Triplet>& faceIDs)
        : IShape{ShapeType::Trimesh}
        , _vertices{vertices}
        , _faces{faceIDs} {
    }

    Trimesh::Trimesh(const Trimesh& trimesh)
        : Trimesh{} {
        this->operator=(trimesh);
    }

    Trimesh::Trimesh(Trimesh&& trimesh)
        : Trimesh{} {
        this->operator=(std::move(trimesh));
    }

    Trimesh::~Trimesh() {
    }

    Trimesh& Trimesh::operator=(const Trimesh& trimesh) {
        _vertices   = trimesh._vertices;
        _faces      = trimesh._faces;
        _accel      = trimesh._accel;
        _texture    = trimesh._texture;
        return *this;
    }

    Trimesh& Trimesh::operator=(Trimesh&& trimesh) {
        _vertices = std::move(trimesh._vertices);
        _faces    = std::move(trimesh._faces);
        _accel    = trimesh._accel;
        _texture  = trimesh._texture;
        return *this;    
    }

    bool Trimesh::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        Assertion(_accel, "Accelerator is not constructed");
        hitpoint->setDistance(INFTY);
        return _accel.intersect(ray, hitpoint) != -1;
    }

    double Trimesh::area() const {
        double ret = 0.0;
        for (unsigned int i = 0; i < _faces.size(); i++) {
            Triangle tri = this->getTriangle(i);
            ret += tri.area();
        }
        return ret;
    }

    Trimesh Trimesh::triangulate() const {
        return *this;
    }

    void Trimesh::buildAccel(AccelType accelType) {
        std::vector<Triangle> triangles(_faces.size());
        for (unsigned int i = 0; i < _faces.size(); i++) {
            Vector3D p0 = _vertices[_faces[i][0]].pos();
            Vector3D p1 = _vertices[_faces[i][1]].pos();
            Vector3D p2 = _vertices[_faces[i][2]].pos();
            triangles[i] = Triangle(p0, p1, p2);
        }

        _accel.setAccelType(accelType);
        _accel.construct(triangles);
    }

    void Trimesh::resize(int numVerts, int numFaces) {
        this->_vertices.resize(numVerts);
        this->_faces.resize(numFaces);
    }

    void Trimesh::calcVertexNormals() {
        const int numVerts = static_cast<int>(_vertices.size());
        const int numFaces = static_cast<int>(_faces.size());
        std::vector<int> faceCount(numVerts, 0);
        std::vector<Vector3D> normals(numVerts, Vector3D(0.0, 0.0, 0.0));
        for (int i = 0; i < numFaces; i++) {
            const Vector3D v0  = _vertices[_faces[i][0]].pos();
            const Vector3D v1  = _vertices[_faces[i][1]].pos();
            const Vector3D v2  = _vertices[_faces[i][2]].pos();
            const Vector3D nrm = Triangle(v0, v1, v2).normal();
            for (int k = 0; k < 3; k++) {
                normals[_faces[i][k]] += nrm;
                faceCount[_faces[i][k]] += 1;
            }
        }

        for (int i = 0; i < numVerts; i++) {
            _vertices[i].setNormal(normals[i] / (faceCount[i] + EPS));
        }
    }

    void Trimesh::load(const std::string& filename) {
        std::string ext = path::getExtension(filename);

        std::unique_ptr<MeshIO> meshio;
        if (ext == ".ply") {
            meshio = std::make_unique<PLYMeshIO>();
        } else if (ext == ".obj") {
            meshio = std::make_unique<OBJMeshIO>();
        } else {
            std::cerr << "[ERROR] unsupported mesh file extension: " << ext << std::endl;
            std::abort();
        }

        meshio->load(filename, this);
        calcVertexNormals();
    }

    void Trimesh::translate(const Vector3D& move) {
        for (unsigned int i = 0; i < _vertices.size(); i++) {
            _vertices[i].setPosition(_vertices[i].pos() + move);
        }
    }

    void Trimesh::scale(const double scaleX, const double scaleY, const double scaleZ) {
        for (unsigned int i = 0; i < _vertices.size(); i++) {
            const double newx = _vertices[i].pos().x() * scaleX;
            const double newy = _vertices[i].pos().y() * scaleY;
            const double newz = _vertices[i].pos().z() * scaleZ;
            _vertices[i].setPosition(Vector3D(newx, newy, newz));
        }
    }

    void Trimesh::scale(const double scaleAll) {
        scale(scaleAll, scaleAll, scaleAll);
    }

    void Trimesh::rotate(double theta, const Vector3D& axis,
                         const Vector3D& origin) {
        translate(-origin);                
        for (int i = 0; i < _vertices.size(); i++) {
            const Vector3D& pos = _vertices[i].pos();
            const Vector3D& nrm = _vertices[i].normal();
            Quaternion rot = Quaternion::rotation(axis, theta);
            _vertices[i].setPosition(rot.applyTo(pos));
            _vertices[i].setNormal(rot.applyTo(nrm));
        }
        translate(origin);
    }

    void Trimesh::putOnPlane(const Plane& plane) {
        // Find nearest point
        double minval = INFTY;
        for (size_t i = 0; i < _vertices.size(); i++) {
            const double dt = Vector3D::dot(plane.normal(), _vertices[i].pos());
            minval = std::min(minval, dt);
        }

        for (size_t i = 0; i < _vertices.size(); i++) {
            const Vector3D newPos = _vertices[i].pos() - 
                                    (minval + plane.distance()) * plane.normal();
            _vertices[i].setPosition(newPos);
        }
    }

    void Trimesh::fitToBBox(const BBox& bbox) {
        BBox orgBox;
        for (size_t i = 0; i < _vertices.size(); i++) {
            orgBox.merge(_vertices[i].pos());
        }

        const Vector3D targetSize = bbox.posMax() - bbox.posMin();
        const Vector3D orgSize = orgBox.posMax() - orgBox.posMin();

        const double scaleX = targetSize.x() / orgSize.x();
        const double scaleY = targetSize.y() / orgSize.y();
        const double scaleZ = targetSize.z() / orgSize.z();
        const double scaleAll = std::min(scaleX, std::min(scaleY, scaleZ));
        this->scale(scaleAll);

        const Vector3D prevCenter = (orgBox.posMin() + orgBox.posMax()) * (0.5 * scaleAll);
        const Vector3D toCenter = (bbox.posMin() + bbox.posMax()) * 0.5;
        this->translate(toCenter - prevCenter);
    }

    const std::vector<Triplet>& Trimesh::getIndices() const {
        return _faces;
    }

    const Vector3D& Trimesh::getVertex(int id) const {
        Assertion(id >= 0 && id < _vertices.size(), "Vertex index out of bounds");
        return _vertices[id].pos();
    }

    void Trimesh::setVertex(int id, const Vector3D& v) {
        Assertion(id >= 0 && id < _vertices.size(), "Vertex index out of bounds!!");
        _vertices[id].setPosition(v);
    }

    const VertexData& Trimesh::getVertexData(int id) const {
        Assertion(id >= 0 && id < _vertices.size(), "Vertex index out of bounds");
        return _vertices[id];    
    }

    void Trimesh::setFace(int id, const Triplet& face) {
        Assertion(id >= 0 && id < _faces.size(), "Face index out of bounds!!");
        _faces[id] = face;
    }

    const Vector2D& Trimesh::getTexcoord(int id) const {
        Assertion(id >= 0 && id < _vertices.size(), "Vertex index out of bounds");
        return _vertices[id].texcoord();
    }

    void Trimesh::setTexcoord(int id, const Vector2D& texcoord) {
        Assertion(id >= 0 && id < _vertices.size(), "Vertex index out of bounds!!");
        return _vertices[id].setTexcoord(texcoord);
    }

    void Trimesh::setColor(int id, const Color& color) {
        Assertion(id >= 0 && id < _vertices.size(), "Vertex index out of bounds");
        _vertices[id].setColor(color);
    }

    Color Trimesh::getColor(int id) const {
        Assertion(id >= 0 && id < _vertices.size(), "Vertex index out of bounds");
        return _vertices[id].color();
    }

    Vector3D Trimesh::getNormal(int id) const {
        Assertion(id >= 0 && id < _vertices.size(), "Vertex index out of bounds");
        return _vertices[id].normal();
    }

    Triangle Trimesh::getTriangle(int id) const {
        Assertion(id >= 0 && id < _faces.size(), "Triangle index out of bounds");
        const Vector3D& p0 = _vertices[_faces[id][0]].pos();
        const Vector3D& p1 = _vertices[_faces[id][1]].pos();
        const Vector3D& p2 = _vertices[_faces[id][2]].pos();
        return Triangle(p0, p1, p2);
    }

    std::vector<Triangle> Trimesh::getTriangleList() const {
        std::vector<Triangle> tris(_faces.size());
        for (int i = 0; i < _faces.size(); i++) {
            tris[i] = getTriangle(i);
        }
        return std::move(tris);
    }

    void Trimesh::setTexture(const Image& image) {
        _texture = std::make_shared<Image>(image);
    }

}  // namesapce spica
