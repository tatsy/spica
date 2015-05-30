#define SPICA_TRIMESH_EXPORT
#include "trimesh.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "../utils/common.h"

namespace spica {

    Trimesh::Trimesh()
        : _numVerts(0)
        , _numFaces(0)
        , _vertices(NULL)
        , _faces(NULL)
        , _normals(NULL)
        , _accel(NULL)
        , _accelType(QBVH_ACCEL)
    {
    }

    Trimesh::Trimesh(const std::string& filename)
        : _numVerts(0)
        , _numFaces(0)
        , _vertices(NULL)
        , _faces(NULL)
        , _normals(NULL)
        , _accel(NULL)
        , _accelType(QBVH_ACCEL)
    {
        load(filename);
    }

    Trimesh::Trimesh(const Trimesh& trimesh)
        : _numVerts(0)
        , _numFaces(0)
        , _vertices(NULL)
        , _faces(NULL)
        , _normals(NULL)
        , _accel(NULL)
        , _accelType(QBVH_ACCEL)
    {
        operator=(trimesh);
    }

    Trimesh::~Trimesh()
    {
        release();
    }

    void Trimesh::release() {
        delete[] _vertices;
        delete[] _faces;
        delete[] _normals;
        delete   _accel;
        _numVerts = 0;
        _numFaces = 0;
        _vertices = NULL;
        _faces    = NULL;
        _normals  = NULL;
        _accel    = NULL;
    }

    Trimesh& Trimesh::operator=(const Trimesh& trimesh) {
        release();

        _numVerts = trimesh._numVerts;
        _numFaces = trimesh._numFaces;
        _vertices = new Vector3[trimesh._numVerts];
        _faces = new int[trimesh._numFaces * 3];
        _normals = new Vector3[trimesh._numFaces];
        _accel = NULL;
        _accelType = trimesh._accelType;
        
        memcpy(_vertices, trimesh._vertices, sizeof(Vector3) * _numVerts);
        memcpy(_faces, trimesh._faces, sizeof(int) * (_numFaces * 3));
        memcpy(_normals, trimesh._normals, sizeof(Vector3) * _numFaces);

        buildAccel();

        return *this;
    }

    bool Trimesh::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        msg_assert(_accel != NULL, "Accelerator is not constructed");
        hitpoint->setDistance(INFTY);
        return _accel->intersect(ray, hitpoint);
    }

    double Trimesh::area() const {
        double ret = 0.0;
        for (int i = 0; i < _numFaces; i++) {
            Triangle tri = this->getTriangle(i);
            ret += tri.area();
        }
        return ret;
    }

    void Trimesh::setAccelType(AccelType accelType, bool doBuild) {
        this->_accelType = accelType;
        if (doBuild) {
            buildAccel();
        }
    }

    void Trimesh::buildAccel() {
        std::vector<Triangle> triangles(_numFaces);
        for (int i = 0; i < _numFaces; i++) {
            Vector3& p0 = _vertices[_faces[i * 3 + 0]];
            Vector3& p1 = _vertices[_faces[i * 3 + 1]];
            Vector3& p2 = _vertices[_faces[i * 3 + 2]];
            triangles[i] = Triangle(p0, p1, p2);
        }

        switch(_accelType) {
        case KD_TREE_ACCEL:
            printf("Accelerator: K-D tree\n");
            _accel = new KdTreeAccel();
            break;
        case QBVH_ACCEL:
            printf("Accelerator: QBVH\n");
            _accel = new QBVHAccel();
            break;
        default:
            msg_assert(false, "Unknown accelerator type!!");
            break;
        }
        _accel->construct(triangles);
    }

    void Trimesh::load(const std::string& filename) {
        release();

        int dotPos = filename.find_last_of(".");
        std::string ext = filename.substr(dotPos);
        
        if (ext == ".ply") {
            this->loadPly(filename);
        } else if (ext == ".obj") {
            this->loadObj(filename);
        } else {
            msg_assert(ext == ".ply" || ext == ".obj", "Mesh loader only accepts .ply and .obj file format");
        }
    }

    void Trimesh::loadPly(const std::string& filename) {
        std::ifstream in(filename.c_str(), std::ios::in);
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
                    // std::cout << key << " " << name << " " << val << std::endl;
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

                    _normals[i] = Vector3::cross(_vertices[p1] - _vertices[p0], _vertices[p2] - _vertices[p0]).normalized();
                }
                break;
            }
        }
    }

    void Trimesh::loadObj(const std::string& filename) {
        std::ifstream in(filename.c_str(), std::ios::in);
        msg_assert(in.is_open(), "Failed to open mesh file");

        std::string typ;
        this->_numVerts = 0;
        this->_numFaces = 0;
        std::vector<Vector3> verts;
        std::vector<int> faces;
        while (!in.eof()) {
            in >> typ;
            if (typ == "v") {
                _numVerts++;
                
                double x, y, z;
                in >> x >> y >> z;
                verts.emplace_back(x, y, z);
            } else if (typ == "f") {
                _numFaces++;

                int v0, v1, v2;
                in >> v0 >> v1 >> v2;
                faces.push_back(v0 - 1);
                faces.push_back(v1 - 1);
                faces.push_back(v2 - 1);
            } else {
                msg_assert(false, "Unknown type is found while reading .obj file!!");
            }
        }

        _vertices = new Vector3[_numVerts];
        _faces    = new int[_numFaces * 3];
        _normals  = new Vector3[_numFaces];
        memcpy(_vertices, &verts[0], sizeof(Vector3) * _numVerts);
        memcpy(_faces, &faces[0], sizeof(int) * _numFaces * 3);
        for (int i = 0; i < _numFaces; i++) {
            int p0 = _faces[i * 3 + 0];
            int p1 = _faces[i * 3 + 1];
            int p2 = _faces[i * 3 + 2];
            _normals[i] = Vector3::cross(_vertices[p1] - _vertices[p0], _vertices[p2] - _vertices[p0]);
        }
    }

    void Trimesh::translate(const Vector3& move) {
        for (int i = 0; i < _numVerts; i++) {
            _vertices[i] += move;
        }
    }

    void Trimesh::scale(const double scaleX, const double scaleY, const double scaleZ) {
        for (int i = 0; i < _numVerts; i++) {
            _vertices[i].setX(_vertices[i].x() * scaleX);
            _vertices[i].setY(_vertices[i].y() * scaleY);
            _vertices[i].setZ(_vertices[i].z() * scaleZ);
        }
    }

    void Trimesh::putOnPlane(const Plane& plane) {
        // Find nearest point
        double minval = INFTY;
        for (int i = 0; i < _numVerts; i++) {
            minval = std::min(minval, Vector3::dot(plane.normal(), _vertices[i]));
        }

        for (int i = 0; i < _numVerts; i++) {
            _vertices[i] -= (minval + plane.distance()) * plane.normal();
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
