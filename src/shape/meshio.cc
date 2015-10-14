#define SPICA_MESHIO_EXPORT
#include "meshio.h"

#include <iostream>
#include <fstream>

#include "../core/common.h"
#include "../core/image.h"
#include "../core/path.h"

#include "trimesh.h"

namespace spica {

    PLYMeshIO::PLYMeshIO()
        : MeshIO{} {
    }

    PLYMeshIO::~PLYMeshIO() {
    }

    void PLYMeshIO::load(const std::string& filename, Trimesh* trimesh) const {
        std::ifstream ifs(filename.c_str(),
                          std::ios::in | std::ios::binary);

        if (!ifs.is_open()) {
            std::cerr << "[ERROR] failed to open file \""
                      << filename << "\"!!" << std::endl;
            std::abort();
        }

        std::string line, format, key, name, val;
        int numVerts = 0;
        int numFaces = 0;

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
                        ss >> val;
                        std::string dir = path::getDirectory(filename);
                        std::string imgfile = dir + val;
                        trimesh->setTexture(Image::fromFile(imgfile));
                    }
                } else {
                    continue;
                }
            } else {
                Assertion(numVerts > 0 && numFaces > 0, "numVerts and numFaces must be positive");

                trimesh->resize(numVerts, numFaces);

                float ff[3];
                float tt[2];
                for (size_t i = 0; i < numVerts; i++) {
                    ifs.read((char*)ff, sizeof(float) * 3);
                    trimesh->setVertex(i, Vector3D(ff[0], ff[1], ff[2]));
                    
                    if (trimesh->isTextured()) {
                        ifs.read((char*)tt, sizeof(float) * 2);
                        trimesh->setTexcoord(i, Vector2D(tt[0], tt[1]));
                    }
                }

                unsigned char vs;
                int ii[3];
                for (size_t i = 0; i < numFaces; i++) {
                    ifs.read((char*)&vs, sizeof(unsigned char));
                    ifs.read((char*)ii, sizeof(int) * 3);
                    trimesh->setFace(i, Triplet(ii[0], ii[1], ii[2]));
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
    }

    void PLYMeshIO::save(const std::string& filename,
                         const Trimesh& trimesh) const {
        std::cerr << "[ERROR] not implemented yet" << std::endl;
        std::abort();
    }



    OBJMeshIO::OBJMeshIO()
        : MeshIO{} {
    }

    OBJMeshIO::~OBJMeshIO() {
    }

    void OBJMeshIO::load(const std::string& filename, Trimesh* trimesh) const {
    std::ifstream ifs(filename.c_str(), std::ios::in);
        Assertion(ifs.is_open(), "Failed to open mesh file");

        std::stringstream ss;
        std::string line;

        std::vector<Vector3D> vertices;
        std::vector<Triplet> faces;
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

            if (typ == "v") {               
                double x, y, z;
                ss >> x >> y >> z;
                vertices.emplace_back(x, y, z);
            } else if (typ == "f") {
                int v0, v1, v2;
                ss >> v0 >> v1 >> v2;
                faces.emplace_back(v0 - 1, v1 - 1, v2 - 1);
            } else {
                char msg[256];
                sprintf(msg, "Unknown type \"%s\" is found while reading .obj file!!", typ.c_str());
                Assertion(false, msg);
            }
        }
        ifs.close();

        (*trimesh) = Trimesh(vertices, faces);
    }

    void OBJMeshIO::save(const std::string& filename,
                         const Trimesh& trimesh) const {
        std::cerr << "[ERROR] not implemented yet" << std::endl;
        std::abort();    
    }

}  // namespace spica
