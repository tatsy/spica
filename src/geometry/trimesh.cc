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
        : _vertices()
        , _colors()
        , _faces()
        , _normals()
        , _accel(NULL)
        , _accelType(QBVH_ACCEL)
    {
    }

    Trimesh::Trimesh(const std::string& filename)
        : _vertices()
        , _colors()
        , _faces()
        , _normals()
        , _accel(NULL)
        , _accelType(QBVH_ACCEL)
    {
        load(filename);
    }

    Trimesh::Trimesh(const std::vector<Vector3>& vertices, const std::vector<Triplet>& faceIDs) 
        : _vertices(vertices)
        , _colors()
        , _faces(faceIDs)
        , _normals()
        , _accel(NULL)
        , _accelType(QBVH_ACCEL)
    {
        _colors.resize(_vertices.size());
        _normals.resize(_vertices.size());

        for (unsigned int i = 0; i < _faces.size(); i++) {
            const Vector3& v0 = _vertices[_faces[i][0]];
            const Vector3& v1 = _vertices[_faces[i][1]];
            const Vector3& v2 = _vertices[_faces[i][2]];
            _colors[i]  = Color(0.0, 0.0, 0.0);
            _normals[i] = Vector3::cross(v1 - v0, v2 - v0).normalized();
        }
    }

    Trimesh::Trimesh(const Trimesh& trimesh)
        : _vertices()
        , _colors()
        , _faces()
        , _normals()
        , _accel(NULL)
        , _accelType(QBVH_ACCEL)
    {
        this->operator=(trimesh);
    }

    Trimesh::Trimesh(Trimesh&& trimesh)
        : _vertices()
        , _colors()
        , _faces()
        , _normals()
        , _accel(NULL)
        , _accelType(QBVH_ACCEL)
    {
        this->operator=(std::move(trimesh));
    }

    Trimesh::~Trimesh()
    {
    }

    Trimesh& Trimesh::operator=(const Trimesh& trimesh) {
        _vertices = trimesh._vertices;
        _colors   = trimesh._colors;
        _faces    = trimesh._faces;
        _normals  = trimesh._normals;
        _accel    = trimesh._accel;
        _accelType = trimesh._accelType;
        return *this;
    }

    Trimesh& Trimesh::operator=(Trimesh&& trimesh) {
        _vertices = std::move(trimesh._vertices);
        _colors   = std::move(trimesh._colors);
        _faces    = std::move(trimesh._faces);
        _normals  = std::move(trimesh._normals);
        _accel    = trimesh._accel;
        _accelType = trimesh._accelType;
        return *this;    
    }

    bool Trimesh::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        msg_assert(_accel != NULL, "Accelerator is not constructed");
        hitpoint->setDistance(INFTY);
        return _accel->intersect(ray, hitpoint);
    }

    double Trimesh::area() const {
        double ret = 0.0;
        for (unsigned int i = 0; i < _faces.size(); i++) {
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
        std::vector<Triangle> triangles(_faces.size());
        for (unsigned int i = 0; i < _faces.size(); i++) {
            Vector3& p0 = _vertices[_faces[i][0]];
            Vector3& p1 = _vertices[_faces[i][1]];
            Vector3& p2 = _vertices[_faces[i][2]];
            triangles[i] = Triangle(p0, p1, p2);
        }

        switch(_accelType) {
        case KD_TREE_ACCEL:
            printf("Accelerator: K-D tree\n");
            _accel = std::shared_ptr<AccelBase>(new KdTreeAccel());
            break;
        case QBVH_ACCEL:
            printf("Accelerator: QBVH\n");
            _accel = std::shared_ptr<AccelBase>(new QBVHAccel());
            break;
        default:
            msg_assert(false, "Unknown accelerator type!!");
            break;
        }
        _accel->construct(triangles);
    }

    void Trimesh::load(const std::string& filename) {
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
        std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
        msg_assert(in.is_open(), "Failed to open mesh file");

        std::string line, format, key, name, val;
        size_t numVerts = 0;
        size_t numFaces = 0;

        std::getline(in, format);
        msg_assert(format == "ply", "Invalid format identifier");

        bool isBody = false;
        while(!in.eof()) {
            if (!isBody) {
                std::getline(in, line);
                std::stringstream ss;
                ss << line;

                ss >> key;
                if (key == "format") {
                    ss >> name >> val;
                    msg_assert(name == "binary_little_endian", "PLY must be binary little endian format!");
                } else if (key == "property") {
                    ss >> name >> val;
                } else if (key == "element") {
                    ss >> name;
                    if (name == "vertex") {
                        ss >> numVerts;
                    } else if (name == "face") {
                        ss >> numFaces;
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
                msg_assert(numVerts > 0 && numFaces > 0, "numVerts and numFaces must be positive");

                _vertices.resize(numVerts);
                _colors.resize(numVerts);
                _faces.resize(numFaces);
                _normals.resize(numFaces);

                int cnt = 0;
                float ff[3];
                for (size_t i = 0; i < numVerts; i++) {
                    in.read((char*)ff, sizeof(float) * 3);
                    _vertices[i] = Vector3(ff[0], ff[1], ff[2]);
                }

                unsigned char vs;
                int ii[3];
                for (size_t i = 0; i < numFaces; i++) {
                    in.read((char*)&vs, sizeof(unsigned char));
                    in.read((char*)ii, sizeof(int) * 3);
                    _faces[i] = Triplet(ii[0], ii[1], ii[2]);

                    if (vs > 3) {
                        printf("[WARNING] mesh contains non-triangle polygon (%d vertices) !!\n", (int)vs);
                        exit(1);
                        in.seekg(sizeof(int) * (vs - 3), std::ios_base::cur);
                    }

                    const int p0 = ii[0];
                    const int p1 = ii[1];
                    const int p2 = ii[2];
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
        _vertices.clear();
        _faces.clear();
        while (!in.eof()) {
            in >> typ;
            if (typ == "v") {               
                double x, y, z;
                in >> x >> y >> z;
                _vertices.push_back(Vector3(x, y, z));
            } else if (typ == "f") {
                int v0, v1, v2;
                in >> v0 >> v1 >> v2;
                _faces.push_back(Triplet(v0, v1, v2));
            } else {
                msg_assert(false, "Unknown type is found while reading .obj file!!");
            }
        }

        _colors.assign(_vertices.size(), Color(0.0, 0.0, 0.0));
        _normals.resize(_faces.size());

        for (int i = 0; i < _faces.size(); i++) {
            const int p0 = _faces[i][0];
            const int p1 = _faces[i][1];
            const int p2 = _faces[i][2];
            _normals[i] = Vector3::cross(_vertices[p1] - _vertices[p0], _vertices[p2] - _vertices[p0]);
        }
    }

    void Trimesh::translate(const Vector3& move) {
        for (unsigned int i = 0; i < _vertices.size(); i++) {
            _vertices[i] += move;
        }
    }

    void Trimesh::scale(const double scaleX, const double scaleY, const double scaleZ) {
        for (unsigned int i = 0; i < _vertices.size(); i++) {
            _vertices[i].x() = _vertices[i].x() * scaleX;
            _vertices[i].y() = _vertices[i].y() * scaleY;
            _vertices[i].z() = _vertices[i].z() * scaleZ;
        }
    }

    void Trimesh::scale(const double scaleAll) {
        scale(scaleAll, scaleAll, scaleAll);
    }

    void Trimesh::putOnPlane(const Plane& plane) {
        // Find nearest point
        double minval = INFTY;
        for (size_t i = 0; i < _vertices.size(); i++) {
            minval = std::min(minval, Vector3::dot(plane.normal(), _vertices[i]));
        }

        for (size_t i = 0; i < _vertices.size(); i++) {
            _vertices[i] -= (minval + plane.distance()) * plane.normal();
        }
    }

    void Trimesh::fitToBBox(const BBox& bbox) {
        BBox orgBox;
        for (size_t i = 0; i < _vertices.size(); i++) {
            orgBox.merge(_vertices[i]);
        }

        const Vector3 targetSize = bbox.posMax() - bbox.posMin();
        const Vector3 orgSize = orgBox.posMax() - orgBox.posMin();

        const double scaleX = targetSize.x() / orgSize.x();
        const double scaleY = targetSize.y() / orgSize.y();
        const double scaleZ = targetSize.z() / orgSize.z();
        const double scaleAll = std::min(scaleX, std::min(scaleY, scaleZ));
        this->scale(scaleAll);

        const Vector3 prevCenter = (orgBox.posMin() + orgBox.posMax()) * (0.5 * scaleAll);
        const Vector3 toCenter = (bbox.posMin() + bbox.posMax()) * 0.5;
        this->translate(toCenter - prevCenter);
    }

    std::vector<Triplet> Trimesh::getIndices() const {
        std::vector<Triplet> ret(_faces.size());
        std::copy(_faces.begin(), _faces.end(), ret.begin());
        return std::move(ret);
    }

    Vector3 Trimesh::getVertex(int id) const {
        msg_assert(id >= 0 && id < _vertices.size(), "Vertex index out of bounds");
        return _vertices[id];
    }

    void Trimesh::setColor(int id, const Color& color) {
        _colors[id] = color;
    }

    Color Trimesh::getColor(int id) const {
        msg_assert(id >= 0 && id < _vertices.size(), "Vertex index out of bounds");
        return _colors[id];
    }

    Vector3 Trimesh::getNormal(int id) const {
        msg_assert(id >= 0 && id < _faces.size(), "Triangle index out of bounds");
        return _normals[id];
    }

    Triangle Trimesh::getTriangle(int id) const {
        msg_assert(id >= 0 && id < _faces.size(), "Triangle index out of bounds");
        const Vector3& p0 = _vertices[_faces[id][0]];
        const Vector3& p1 = _vertices[_faces[id][1]];
        const Vector3& p2 = _vertices[_faces[id][2]];
        return Triangle(p0, p1, p2);
    }

}  // namesapce spica
