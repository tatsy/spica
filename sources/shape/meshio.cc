#define SPICA_API_EXPORT
#include "meshio.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "../core/common.h"
#include "../core/triplet.h"
#include "../core/path.h"
#include "../shape/triangle.h"
#include "../math/vector2d.h"
#include "../core/point3d.h"

#include "../image/image.h"

namespace spica {

PLYMeshIO::PLYMeshIO()
    : MeshIO{} {
}

PLYMeshIO::~PLYMeshIO() {
}

std::vector<std::shared_ptr<Shape>> PLYMeshIO::load(const std::string& filename,
                                                    const Transform& objectToWorld) const {
    std::ifstream ifs(filename.c_str(),
                      std::ios::in | std::ios::binary);

    if (!ifs.is_open()) {
        std::cerr << "[ERROR] failed to open file \""
                    << filename << "\" !!" << std::endl;
        std::abort();
    }

    std::string line, format, key, name, val;
    int numVerts = 0;
    int numFaces = 0;

    std::getline(ifs, format);
    Assertion(format == "ply", "Invalid format identifier");

    bool isBody = false;
    std::vector<std::shared_ptr<Shape>> tris;
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
                    ss >> val;
                    std::string dir = path::getDirectory(filename);
                    std::string imgfile = dir + val;
                    // trimesh->setTexture(Image::fromFile(imgfile));
                }
            } else {
                continue;
            }
        } else {
            Assertion(numVerts > 0 && numFaces > 0, "numVerts and numFaces must be positive");

            float ff[3];
            float tt[2];
            std::vector<Point3d> vertices;
            for (size_t i = 0; i < numVerts; i++) {
                ifs.read((char*)ff, sizeof(float) * 3);
                vertices.emplace_back(ff[0], ff[1], ff[2]);
                    
                //if (trimesh->isTextured()) {
                //    ifs.read((char*)tt, sizeof(float) * 2);
                //    trimesh->setTexcoord(i, Vector2d(tt[0], tt[1]));
                //}
            }

            unsigned char vs;
            int ii[3];
            for (size_t i = 0; i < numFaces; i++) {
                ifs.read((char*)&vs, sizeof(unsigned char));
                ifs.read((char*)ii, sizeof(int) * 3);
                tris.emplace_back(new Triangle(vertices[ii[0]], vertices[ii[1]], vertices[ii[2]], objectToWorld));
                if (vs > 3) {
                    Warning("[WARNING] mesh contains non-triangle polygon (%d vertices) !!", (int)vs);
                    ifs.seekg(sizeof(int) * (vs - 3), std::ios_base::cur);
                }
            }
            break;
        }
    }
    ifs.close(); 

    Bounds3d b;
    for (const auto& t : tris) {
        b.merge(t->worldBound());
    }
    std::cout << b.posMin() << std::endl;
    std::cout << b.posMax() << std::endl;

    return std::move(tris);
}

void PLYMeshIO::save(const std::string& filename,
                     const std::vector<Triangle>& trimesh) const {
    std::cerr << "[ERROR] not implemented yet" << std::endl;
    std::abort();
}



OBJMeshIO::OBJMeshIO()
    : MeshIO{} {
}

OBJMeshIO::~OBJMeshIO() {
}

std::vector<std::shared_ptr<Shape>> OBJMeshIO::load(const std::string& filename,
                                                    const Transform& objectToWorld) const {
    std::ifstream ifs(filename.c_str(), std::ios::in);
    Assertion(ifs.is_open(), "Failed to open mesh file");

    std::stringstream ss;
    std::string line;

    std::vector<Point3d> vertices;
    std::vector<Vector2d> texcoords;
    std::vector<Triplet>  vertIDs;
    std::vector<Triplet>  texIDs;
    bool hasTexture = false;
    Image texture;
    while (!ifs.eof()) {
        std::getline(ifs, line);

        // Check first character
        auto it = line.begin();
        while (it != line.end() && *it == ' ') ++it;
                
        if(it == line.end() || *it == '#') continue;

        ss.clear();
        ss << line;

        std::string typ;
        ss >> typ;

        if (typ == "mtllib") {
            // Load material file
            std::string mtlfile;
            std::string dir = path::getDirectory(filename);
            ss >> mtlfile;
            texture = getTexture(dir + mtlfile);
            hasTexture = true;
        } else if (typ[0] == 'v') {
            if (typ.size() == 1) {
                double x, y, z;
                ss >> x >> y >> z;
                vertices.emplace_back(x, y, z);
            } else if(typ[1] == 't') {
                double x, y;
                ss >> x >> y;
                texcoords.emplace_back(x, y);
            } else if(typ[1] == 'n') {
                // Normal not supported
            } else {
                FatalError("Unexpected character detected!!");
            }
        } else if (typ[0] == 'f') {
            if (!hasTexture) {
                int v0, v1, v2;
                ss >> v0 >> v1 >> v2;
                vertIDs.emplace_back(v0 - 1, v1 - 1, v2 - 1);
            } else {
                std::string s0, s1, s2;
                ss >> s0 >> s1 >> s2;
                   
                int v0, v1, v2;
                int t0, t1, t2;
                if (sscanf(s0.c_str(), "%d/%d", &v0, &t0) == 2 &&
                    sscanf(s1.c_str(), "%d/%d", &v1, &t1) == 2 &&
                    sscanf(s2.c_str(), "%d/%d", &v2, &t2) == 2) {
                    vertIDs.emplace_back(v0 - 1, v1 - 1, v2 - 1);
                    texIDs.emplace_back(t0 - 1, t1 - 1, t2 - 1);
                } else {
                    FatalError("Unsupported face format!!");
                }
            }
        } else {
            FatalError("Unknown type \"%s\" is found while reading .obj file!!", typ.c_str());
        }
    }
    ifs.close();

    /*
    if (!hasTexture) {
        (*trimesh) = Trimesh(vertices, vertIDs);
    } else {
        trimesh->resize(vertIDs.size() * 3, vertIDs.size());
        for (int i = 0; i < vertIDs.size(); i++) {
            for (int k = 0; k < 3; k++) {
                trimesh->setVertex(i * 3 + k, vertices[vertIDs[i][k]]);
                trimesh->setTexcoord(i * 3 + k, texcoords[texIDs[i][k]]);
            }
            trimesh->setFace(i, Triplet(i * 3 + 0, i * 3 + 1, i * 3 + 2));
        }
        trimesh->setTexture(texture);
    }
    */

    std::vector<std::shared_ptr<Shape>> tris;
    for (int i = 0; i < vertIDs.size(); i++) {
        tris.emplace_back(new Triangle(vertices[vertIDs[i][0]], vertices[vertIDs[i][1]], vertices[vertIDs[i][2]]));        
    }
    return std::move(tris);
}

void OBJMeshIO::save(const std::string& filename,
                     const std::vector<Triangle>& trimesh) const {
    std::cerr << "[ERROR] not implemented yet" << std::endl;
    std::abort();
}

Image OBJMeshIO::getTexture(const std::string& filename) {
    std::ifstream ifs(filename.c_str(), std::ios::in);
    if (!ifs.is_open()) {
        FatalError("Failed to open material file!!");
    }

    std::string ident;
    std::string texpath;
    while (ifs >> ident) {
        if (ident == "map_Kd") {
            std::string dir = path::getDirectory(filename);
            ifs >> texpath;
            return std::move(Image::fromFile(dir + texpath));
        }
    }
    FatalError("map_Kd was not detected!!");
    return Image{};
}

}  // namespace spica
