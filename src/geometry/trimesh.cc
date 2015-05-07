#define SPICA_TRIMESH_EXPORT
#include "trimesh.h"

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
        , _vertices(0)
        , _faces(0)
        , _normals(0)
        , _kdtree()
    {
    }

    Trimesh::Trimesh(const std::string& filename)
        : _numVerts(0)
        , _numFaces(0)
        , _vertices(0)
        , _faces(0)
        , _normals(0)
        , _kdtree()
    {
        load(filename);
    }

    Trimesh::Trimesh(const Trimesh& trimesh)
        : _numVerts(0)
        , _numFaces(0)
        , _vertices(0)
        , _faces(0)
        , _normals(0)
        , _kdtree()
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
        _kdtree = trimesh._kdtree;
        
        memcpy(_vertices, trimesh._vertices, sizeof(Vector3) * _numVerts);
        memcpy(_faces, trimesh._faces, sizeof(int) * _numFaces * 3);
        memcpy(_normals, trimesh._normals, sizeof(Vector3) * _numFaces);

        return *this;
    }

    bool Trimesh::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        msg_assert(!_kdtree.empty(), "k-d tree accelator is not prepared.");

        double tMin, tMax;
        KdTreeNode* node = _kdtree.root();
        if (!node->bbox.intersect(ray, &tMin, &tMax)) {
            return false;
        }

        bool ret =  intersectRec(node, ray, hitpoint, tMin, tMax);
        return ret;
    }

    bool Trimesh::intersectRec(KdTreeNode* node, const Ray& ray, Hitpoint* hitpoint, double tMin, double tMax) const {
        if (node->left == NULL || node->right == NULL) {
            int triID = -1;
            for (int i = node->startID; i < node->endID; i++) {
                const Triangle& tri = _kdtree.getTriangle(i);
                Hitpoint hpTemp;
                if (tri.intersect(ray, &hpTemp)) {
                    if (hitpoint->distance() > hpTemp.distance() && Vector3::dot(ray.direction(), tri.normal()) < 0.0) {
                        *hitpoint = hpTemp;
                        triID = i;
                    }
                }
            }

            if (triID != -1) {
                return true;
            }
            return false;
        }

        // Check which child is nearer
        double lMin = INFTY, lMax = INFTY, rMin = INFTY, rMax = INFTY;
        bool isectL = node->left->bbox.intersect(ray, &lMin, &lMax);
        bool isectR = node->right->bbox.intersect(ray, &rMin, &rMax);
        if (!isectL && !isectR) {
            // Intesecting NO children
            return false;
        }

        // Intersecting only one child
        if (isectL && std::abs(lMin - tMin) < EPS && std::abs(lMax - tMax) < EPS) {
            return intersectRec(node->left, ray, hitpoint, lMin, lMax);
        }

        if (isectR && std::abs(rMin - tMin) < EPS && std::abs(rMax - tMax) < EPS) {
            return intersectRec(node->right, ray, hitpoint, rMin, rMax);
        }

        // Intersecting two children
        KdTreeNode *nearer, *farther;
        if (lMin < rMin) {
            nearer = node->left;
            farther = node->right;
        } else {
            nearer = node->right;
            farther = node->left;
            std::swap(lMin, rMin);
            std::swap(lMax, rMax);
        }

        // Check nearer child first
        if (intersectRec(nearer, ray, hitpoint, lMin, lMax)) {
            return true;
        }
        return intersectRec(farther, ray, hitpoint, rMin, rMax);
    }

    double Trimesh::area() const {
        double ret = 0.0;
        for (int i = 0; i < _numFaces; i++) {
            Triangle tri = this->getTriangle(i);
            ret += tri.area();
        }
        return ret;
    }

    void Trimesh::buildKdTreeAccel() {
        std::vector<Triangle> triangles(_numFaces);
        for (int i = 0; i < _numFaces; i++) {
            Vector3& p0 = _vertices[_faces[i * 3 + 0]];
            Vector3& p1 = _vertices[_faces[i * 3 + 1]];
            Vector3& p2 = _vertices[_faces[i * 3 + 2]];
            triangles[i] = Triangle(p0, p1, p2);
        }
        _kdtree.construct(triangles);
    }

    void Trimesh::load(const std::string& filename) {
        int dotPos = filename.find_last_of(".");
        std::string ext = filename.substr(dotPos);
        // std::cout << "Extention: " << ext << std::endl;
        msg_assert(ext == ".ply", "Mesh loader only accepts .ply file format");

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

    void Trimesh::translate(const Vector3& move) {
        for (int i = 0; i < _numVerts; i++) {
            _vertices[i] += move;
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
