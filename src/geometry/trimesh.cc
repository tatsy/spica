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
        , _faces()
        , _accel()
        , _accelType(AccelType::qbvhAccel)
        , _texture()
        , _isTextured(false) {
    }

    Trimesh::Trimesh(const std::string& filename)
        : _vertices()
        , _faces()
        , _accel()
        , _accelType(AccelType::qbvhAccel)
        , _texture()
        , _isTextured(false) {
        load(filename);
    }

    Trimesh::Trimesh(const std::vector<Vector3D>& vertices, const std::vector<Triplet>& faceIDs) 
        : _vertices()
        , _faces(faceIDs)
        , _accel()
        , _accelType(AccelType::qbvhAccel)
        , _texture()
        , _isTextured(false) {
        // Set vertex positions
        _vertices.resize(vertices.size());
        const int nverts = static_cast<int>(vertices.size());
        for (int i = 0; i < nverts; i++) {
            _vertices[i] = VertexData(vertices[i]);
        }

        // Compute vertex normals
        calcVertexNormals();
    }

    Trimesh::Trimesh(const Trimesh& trimesh)
        : _vertices()
        , _faces()
        , _accel()
        , _accelType(AccelType::qbvhAccel)
        , _texture()
        , _isTextured(false) {
        this->operator=(trimesh);
    }

    Trimesh::Trimesh(Trimesh&& trimesh)
        : _vertices()
        , _faces()
        , _accel()
        , _accelType(AccelType::qbvhAccel)
        , _texture()
        , _isTextured(false) {
        this->operator=(std::move(trimesh));
    }

    Trimesh::~Trimesh() {
    }

    Trimesh& Trimesh::operator=(const Trimesh& trimesh) {
        _vertices  = trimesh._vertices;
        _faces     = trimesh._faces;
        _accel     = trimesh._accel;
        _accelType = trimesh._accelType;
        _texture   = trimesh._texture;
        _isTextured = trimesh._isTextured;
        return *this;
    }

    Trimesh& Trimesh::operator=(Trimesh&& trimesh) {
        _vertices  = std::move(trimesh._vertices);
        _faces     = std::move(trimesh._faces);
        _accel     = trimesh._accel;
        _accelType = trimesh._accelType;
        _texture   = trimesh._texture;
        _isTextured = trimesh._isTextured;
        return *this;    
    }

    bool Trimesh::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        Assertion(_accel != NULL, "Accelerator is not constructed");
        hitpoint->setDistance(INFTY);
        return _accel->intersect(ray, hitpoint) != -1;
    }

    double Trimesh::area() const {
        double ret = 0.0;
        for (unsigned int i = 0; i < _faces.size(); i++) {
            Triangle tri = this->getTriangle(i);
            ret += tri.area();
        }
        return ret;
    }

    std::vector<Triangle> Trimesh::triangulate() const {
        std::vector<Triangle> retval;
        for (int i = 0; i < _faces.size(); i++) {
            retval.push_back(getTriangle(i));
        }
        return std::move(retval);
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
            Vector3D p0 = _vertices[_faces[i][0]].pos();
            Vector3D p1 = _vertices[_faces[i][1]].pos();
            Vector3D p2 = _vertices[_faces[i][2]].pos();
            triangles[i] = Triangle(p0, p1, p2);
        }

        switch(_accelType) {
        case AccelType::kdtreeAccel:
            _accel = std::make_shared<KdTreeAccel>();
            break;
        case AccelType::qbvhAccel:
            _accel = std::make_shared<QBVHAccel>();
            break;
        default:
            std::cerr << "Unknown accelerator type!!" << std::endl;
            std::abort();
        }
        _accel->construct(triangles);
    }

    void Trimesh::calcVertexNormals() {
        const int numVerts = static_cast<int>(_vertices.size());
        const int numFaces = static_cast<int>(_faces.size());
        std::vector<int> faceCount(numVerts, 0);
        std::vector<Vector3D> normals(numVerts, Vector3D(0.0, 0.0, 0.0));
        for (int i = 0; i < numFaces; i++) {
            const Vector3D v0 = _vertices[_faces[i][0]].pos();
            const Vector3D v1 = _vertices[_faces[i][1]].pos();
            const Vector3D v2 = _vertices[_faces[i][2]].pos();
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
        std::string ext = "";
        int dotPos = filename.find_last_of(".");
        if (dotPos != std::string::npos) {
            ext = filename.substr(dotPos);
            if (ext == ".ply") {
                this->loadPly(filename);
                return;
            } else if (ext == ".obj") {
                this->loadObj(filename);
                return;
            } 
        }
        Assertion(ext == ".ply" || ext == ".obj", "Mesh loader only accepts .ply and .obj file format");
    }

    void Trimesh::loadPly(const std::string& filename) {
        std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
        Assertion(ifs.is_open(), "Failed to open mesh file");

        std::string line, format, key, name, val;
        size_t numVerts = 0;
        size_t numFaces = 0;

        std::getline(ifs, format);
        Assertion(format == "ply", "Invalid format identifier");

        bool isBody = false;
        while(!ifs.eof()) {
            if (!isBody) {
                std::getline(ifs, line);
                std::stringstream ss;
                ss << line;

                ss >> key;
                if (key == "format") {
                    ss >> name >> val;
                    Assertion(name == "binary_little_endian", "PLY must be binary little endian format!");
                } else if (key == "property") {
                    ss >> name >> val;
                } else if (key == "element") {
                    ss >> name;
                    if (name == "vertex") {
                        ss >> numVerts;
                    } else if (name == "face") {
                        ss >> numFaces;
                    } else {
                        Assertion(false, "Invalid element indentifier");
                    }
                } else if (key == "end_header") {
                    isBody = true;
                    continue;
                } else if (key == "comment") {
                    ss >> name;
                    if (name == "TextureFile") {
                        _isTextured = true;
                        ss >> val;
                        _texture = std::make_shared<Image>(Image::fromFile(val));
                    }
                } else {
                    continue;
                }
            } else {
                Assertion(numVerts > 0 && numFaces > 0, "numVerts and numFaces must be positive");

                _vertices.resize(numVerts);
                _faces.resize(numFaces);

                int cnt = 0;
                float ff[3];
                float tt[2];
                for (size_t i = 0; i < numVerts; i++) {
                    ifs.read((char*)ff, sizeof(float) * 3);
                    _vertices[i] = VertexData(Vector3D(ff[0], ff[1], ff[2]));
                    
                    if (_isTextured) {
                        ifs.read((char*)tt, sizeof(float) * 2);
                        _vertices[i].setTexcoord(Vector2D(tt[0], tt[1]));
                    }
                }


                unsigned char vs;
                int ii[3];
                for (size_t i = 0; i < numFaces; i++) {
                    ifs.read((char*)&vs, sizeof(unsigned char));
                    ifs.read((char*)ii, sizeof(int) * 3);
                    _faces[i] = Triplet(ii[0], ii[1], ii[2]);
                    if (vs > 3) {
                        printf("[WARNING] mesh contains non-triangle polygon (%d vertices) !!\n", (int)vs);
                        exit(1);
                        ifs.seekg(sizeof(int) * (vs - 3), std::ios_base::cur);
                    }
                }
                break;
            }
        }
        ifs.close();

        // Compute vertex normals
        calcVertexNormals();
    }

    void Trimesh::loadObj(const std::string& filename) {
        std::ifstream ifs(filename.c_str(), std::ios::in);
        Assertion(ifs.is_open(), "Failed to open mesh file");

        std::stringstream ss;
        std::string line;
        _vertices.clear();
        _faces.clear();
        while (!ifs.eof()) {
            std::getline(ifs, line);

            // Check first character
            auto it = line.begin();
            while (*it == ' ') ++it;
            if (*it == '#' || it == line.end()) continue;

            ss.clear();
            ss << line;

            std::string typ;
            ss >> typ;

            if (typ == "v") {               
                double x, y, z;
                ss >> x >> y >> z;
                _vertices.emplace_back(Vector3D(x, y, z));
            } else if (typ == "f") {
                int v0, v1, v2;
                ss >> v0 >> v1 >> v2;
                _faces.push_back(Triplet(v0 - 1, v1 - 1, v2 - 1));
            } else {
                char msg[256];
                sprintf(msg, "Unknown type \"%s\" is found while reading .obj file!!", typ.c_str());
                Assertion(false, msg);
            }
        }
        ifs.close();

        // Compute vertex normals
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

    Vector3D Trimesh::getVertex(int id) const {
        Assertion(id >= 0 && id < _vertices.size(), "Vertex index out of bounds");
        return _vertices[id].pos();
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

}  // namesapce spica
