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

    std::vector<Point3d>  vertices;
    std::vector<Point2d> texcoords;
    std::vector<Normal3d> normals;

    std::vector<Triplet> vertIDs;
    std::vector<Triplet> texIDs;
    std::vector<Triplet> nrmIDs;

    bool hasTexture = false;
    Image texture;
    while (!ifs.eof()) {
        std::getline(ifs, line);

        // Check first character
        auto it = line.begin();
        while (it != line.end() && *it == ' ') ++it;
                
        if(it == line.end() || *it == '#') continue;

        ss.clear();
        ss.str(line);

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
                double x, y, z;
                ss >> x >> y >> z;
                normals.emplace_back(x, y, z);
            } else {
                FatalError("Unexpected character detected!!");
            }
        } else if (typ[0] == 'f') {
            char s0[64], s1[64], s2[64], s3[64];
            int res = sscanf(line.c_str(), "f %s %s %s %s", s0, s1, s2, s3);
            if (res == 3) {
                // Triangle mesh
                int v0, v1, v2;
                int n0, n1, n2;
                int t0, t1, t2;
                if (sscanf(s0, "%d/%d/%d", &v0, &t0, &n0) == 3 &&
                    sscanf(s1, "%d/%d/%d", &v1, &t1, &n1) == 3 &&
                    sscanf(s1, "%d/%d/%d", &v2, &t2, &n2) == 3) {
                    // All vertex, normal, texcoord exist
                    vertIDs.emplace_back(v0, v1, v2);
                    texIDs.emplace_back(t0, t1, t2);
                    nrmIDs.emplace_back(n0, n1, n2);
                } else if (sscanf(s0, "%d//%d", &v0, &n0) == 2 &&
                           sscanf(s1, "%d//%d", &v1, &n1) == 2 &&
                           sscanf(s2, "%d//%d", &v2, &n2) == 2) {
                    // Vertex and normal
                    vertIDs.emplace_back(v0, v1, v2);
                    nrmIDs.emplace_back(n0, n1, n2);
                } else if (sscanf(s0, "%d/%d", &v0, &t0) == 2 &&
                           sscanf(s1, "%d/%d", &v1, &t1) == 2 &&
                           sscanf(s2, "%d/%d", &v2, &t2) == 2) {
                    // Vertex and texcoord
                    vertIDs.emplace_back(v0, v1, v2);
                    texIDs.emplace_back(t0, t1, t2);
                } else if (sscanf(s0, "%d", &v0) == 1 &&
                           sscanf(s1, "%d", &v1) == 1 &&
                           sscanf(s2, "%d", &v2) == 1) {
                    // Only vertex
                    vertIDs.emplace_back(v0, v1, v2);
                } else {
                    FatalError("Sorry. Unsupported face description was found!!");
                }
            } else if (res == 4) {
                // Quad mesh
                int v0, v1, v2, v3;
                int n0, n1, n2, n3;
                int t0, t1, t2, t3;
                if (sscanf(s0, "%d/%d/%d", &v0, &t0, &n0) == 3 &&
                    sscanf(s1, "%d/%d/%d", &v1, &t1, &n1) == 3 &&
                    sscanf(s2, "%d/%d/%d", &v2, &t2, &n2) == 3 &&
                    sscanf(s3, "%d/%d/%d", &v3, &t3, &n3) == 3) {
                    // All vertex, normal, texcoord exist
                    vertIDs.emplace_back(v0, v1, v2);
                    vertIDs.emplace_back(v0, v2, v3);
                    texIDs.emplace_back(t0, t1, t2);
                    texIDs.emplace_back(t0, t2, t3);
                    nrmIDs.emplace_back(n0, n1, n2);
                    nrmIDs.emplace_back(n0, n2, n3);
                } else if (sscanf(s0, "%d//%d", &v0, &n0) == 2 &&
                           sscanf(s1, "%d//%d", &v1, &n1) == 2 &&
                           sscanf(s2, "%d//%d", &v2, &n2) == 2 &&
                           sscanf(s3, "%d//%d", &v3, &n3) == 2) {
                    // Vertex and normal
                    vertIDs.emplace_back(v0, v1, v2);
                    vertIDs.emplace_back(v0, v2, v3);
                    nrmIDs.emplace_back(n0, n1, n2);
                    nrmIDs.emplace_back(n0, n2, n3);
                } else if (sscanf(s0, "%d/%d", &v0, &t0) == 2 &&
                           sscanf(s1, "%d/%d", &v1, &t1) == 2 &&
                           sscanf(s2, "%d/%d", &v2, &t2) == 2 &&
                           sscanf(s3, "%d/%d", &v3, &t3) == 2) {
                    // Vertex and texcoord
                    vertIDs.emplace_back(v0, v1, v2);
                    vertIDs.emplace_back(v0, v2, v3);
                    texIDs.emplace_back(t0, t1, t2);
                    texIDs.emplace_back(t0, t2, t3);
                } else if (sscanf(s0, "%d", &v0) == 1 &&
                           sscanf(s1, "%d", &v1) == 1 &&
                           sscanf(s2, "%d", &v2) == 1 &&
                           sscanf(s3, "%d", &v3) == 1) {
                    // Only vertex
                    vertIDs.emplace_back(v0, v1, v2);
                    vertIDs.emplace_back(v0, v2, v3);
                } else {
                    FatalError("Sorry. Unsupported face description was found!!");
                }
            } else {
                FatalError("Mesh is neither triangle nor quadrangle!!");
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
        if (!nrmIDs.empty() && !texIDs.empty()) {
            tris.emplace_back(new Triangle(vertices[vertIDs[i][0] - 1],
                                           vertices[vertIDs[i][1] - 1],
                                           vertices[vertIDs[i][2] - 1],
                                           normals[nrmIDs[i][0] - 1],
                                           normals[nrmIDs[i][1] - 1],
                                           normals[nrmIDs[i][2] - 1],
                                           texcoords[texIDs[i][0] - 1],
                                           texcoords[texIDs[i][1] - 1],
                                           texcoords[texIDs[i][2] - 1],
                                           objectToWorld));        
        } else if (!nrmIDs.empty()) {
            tris.emplace_back(new Triangle(vertices[vertIDs[i][0] - 1],
                                           vertices[vertIDs[i][1] - 1],
                                           vertices[vertIDs[i][2] - 1],      
                                           normals[nrmIDs[i][0] - 1],
                                           normals[nrmIDs[i][1] - 1],
                                           normals[nrmIDs[i][2] - 1],
                                           objectToWorld));
        
        } else if (!texIDs.empty()) {
            /*
            tris.emplace_back(new Triangle(vertices[vertIDs[i][0] - 1],
                                           vertices[vertIDs[i][1] - 1],
                                           vertices[vertIDs[i][2] - 1],  
                                           texcoords[texIDs[i][0] - 1],
                                           texcoords[texIDs[i][1] - 1],
                                           texcoords[texIDs[i][2] - 1],
                                           objectToWorld));        
                                           */
        } else {
            tris.emplace_back(new Triangle(vertices[vertIDs[i][0] - 1],
                                           vertices[vertIDs[i][1] - 1],
                                           vertices[vertIDs[i][2] - 1],
                                           objectToWorld));  
        }
    }
    return std::move(tris);
}

void OBJMeshIO::save(const std::string& filename,
                     const std::vector<Triangle>& trimesh) const {
    std::ofstream ofs(filename, std::ios::out);
    for (const auto& t : trimesh) {
        ofs << "v " << t[0].x() << " " << t[0].y() << " " << t[0].z() << std::endl;
        ofs << "v " << t[1].x() << " " << t[1].y() << " " << t[1].z() << std::endl;
        ofs << "v " << t[2].x() << " " << t[2].y() << " " << t[2].z() << std::endl;
    }

    for (int i = 0; i < trimesh.size(); i++) {
        ofs << "f " << (3 * i + 1) << " " << (i * 3 + 2) << " " << (i * 3 + 3) << std::endl;
    }
    ofs.close();
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
