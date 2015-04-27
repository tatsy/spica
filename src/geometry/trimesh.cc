#define SPICA_TRIMESH_EXPORT
#include "trimesh.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../utils/common.h"

namespace spica {

    Trimesh::Trimesh()
        : Primitive()
        , _vertices(0)
        , _faces(0)
        , _normals(0)
    {
    }

    Trimesh::Trimesh(const std::string& filename, const Material& material)
        : Primitive(material)
        , _vertices(0)
        , _faces(0)
        , _normals(0)
    {
        load(filename);
    }

    Trimesh::Trimesh(const Trimesh& trimesh)
        : Primitive(trimesh)
        , _vertices(0)
        , _faces(0)
        , _normals(0)
    {
        operator=(trimesh);
    }

    Trimesh::~Trimesh()
    {
        delete[] _vertices;
        delete[] _faces;
        delete[] _normals;
    }

    Trimesh& Trimesh::operator=(const Trimesh& trimesh) {
        delete[] _vertices;
        delete[] _faces;
        delete[] _normals;

        _numVerts = trimesh._numVerts;
        _numFaces = trimesh._numFaces;
        _vertices = new Vector3[trimesh._numVerts];
        _faces = new int[trimesh._numFaces * 3];
        _normals = new Vector3[trimesh._numFaces];
        
        memcpy(_vertices, trimesh._vertices, sizeof(Vector3) * _numVerts);
        memcpy(_faces, trimesh._faces, sizeof(int) * _numFaces * 3);
        memcpy(_normals, trimesh._normals, sizeof(Vector3) * _numFaces);

        return *this;
    }

    bool Trimesh::intersect(const Ray& ray, Intersection& intersect) const {
        msg_assert(false, "Not implemented yet");
    }

    void Trimesh::load(const std::string& filename) {
        int dotPos = filename.find_last_of(".");
        std::string ext = filename.substr(dotPos);
        std::cout << "Extention: " << ext << std::endl;
        msg_assert(ext == ".ply", "Mesh loader only accepts .ply file format");

        std::ifstream in(filename, std::ios::in);
        msg_assert(in.is_open(), "Failed to open mesh file");

        std::string format, key, name, val;
        int numVerts = -1;
        int numFaces = -1;

        in >> format;
        msg_assert(format == "ply", "Invalid format identifier");

        bool isBody = false;
        while(!in.eof()) {
            if (!isBody) {
                in >> key;
                if (key == "format" || key == "property") {
                    in >> name >> val;
                    std::cout << key << " " << name << " " << val << std::endl;
                } else if (key == "element") {
                    in >> name;
                    if (name == "vertex") {
                        in >> numVerts;
                    } else if (name == "face") {
                        in >> numFaces;
                    } else {
                        msg_assert(false, "Invalid element indentifier");
                    }
                } else if (key == "end_header") {
                    isBody = true;
                    continue;
                } else {
                    continue;
                }
            } else {
                std::string line;
                std::stringstream ss;
                double vx, vy, vz;
                int nv, p0, p1, p2;

                msg_assert(numVerts > 0 && numFaces > 0, "numVerts and numFaces must be positive");

                _numVerts = numVerts;
                _numFaces = numFaces;
                _vertices = new Vector3[numVerts];
                _faces = new int[numFaces * 3];
                _normals = new Vector3[numFaces];

                std::getline(in, line);  // skip end_header line

                int cnt = 0;
                for (int i = 0; i < numVerts; i++) {
                    std::getline(in, line);
                    ss.str("");
                    ss.clear(std::stringstream::goodbit);
                    ss << line;
                    ss >> vx >> vy >> vz;
                    _vertices[i] = Vector3(vx, vy, vz);
                }

                for (int i = 0; i < numFaces; i++) {
                    std::getline(in, line);
                    ss.str("");
                    ss.clear(std::stringstream::goodbit);
                    ss << line;
                    ss >> nv >> p0 >> p1 >> p2;
                    _faces[i * 3 + 0] = p0;
                    _faces[i * 3 + 1] = p1;
                    _faces[i * 3 + 2] = p2;

                    _normals[i] = Vector3::cross(_vertices[p2] - _vertices[p0], _vertices[p1] - _vertices[p0]).normalized();
                }
                break;
            }
        }
    }

    Vector3 Trimesh::getNormal(int id) const {
        msg_assert(id >= 0 && id < _numFaces, "Triangle index out of bounds");
        return _normals[id];
    }

    Triangle Trimesh::getTriangle(int id) const {
        msg_assert(id >= 0 && id < _numFaces, "Triangle index out of bounds");
        const Vector3& p0 = _vertices[_faces[id * 3 + 0]];
        const Vector3& p1 = _vertices[_faces[id * 3 + 1]];
        const Vector3& p2 = _vertices[_faces[id * 3 + 2]];
        return Triangle(p0, p1, p2);
    }

}  // namesapce spica
