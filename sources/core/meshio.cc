#define SPICA_API_EXPORT
#include "meshio.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "core/common.h"
#include "core/triplet.h"
#include "core/triangle.h"
#include "core/vector2d.h"
#include "core/point3d.h"
#include "core/image.h"
#include "core/mipmap.h"
#include "core/texture.h"
#include "core/imagemap.h"

namespace spica {

// ----------------------------------------------------------------------------
// ShapeGroup method definitions
// ----------------------------------------------------------------------------

ShapeGroup::ShapeGroup()
    : shapes_{} {
}

ShapeGroup::~ShapeGroup() {
}

ShapeGroup::ShapeGroup(
    const std::vector<std::shared_ptr<Shape>>& shapes,
    const std::shared_ptr<Texture<Spectrum>>& mapKd,
    const std::shared_ptr<Texture<double>> & bumpMap)
    : shapes_{ shapes }
    , mapKd_{ mapKd }
    , bumpMap_{ bumpMap } {
}

ShapeGroup::ShapeGroup(const ShapeGroup& sg)
    : ShapeGroup{} {
    this->operator=(sg);
}

ShapeGroup::ShapeGroup(ShapeGroup&& sg)
    : ShapeGroup{} {
    this->operator=(std::move(sg));
}

ShapeGroup& ShapeGroup::operator=(const ShapeGroup& sg) {
    this->shapes_  = sg.shapes_;
    this->mapKd_   = sg.mapKd_;
    this->bumpMap_ = sg.bumpMap_;
    return *this;
}

ShapeGroup& ShapeGroup::operator=(ShapeGroup&& sg) {
    this->shapes_  = std::move(sg.shapes_);
    this->mapKd_   = std::move(sg.mapKd_);
    this->bumpMap_ = std::move(sg.bumpMap_);
    return *this;
}


// ----------------------------------------------------------------------------
// MeshIO method definitions
// ----------------------------------------------------------------------------

namespace meshio {

std::vector<ShapeGroup> loadPLY(const std::string& filename,
                                const Transform& objectToWorld) {
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
                    fs::path path(filename);
                    fs::path fileName(val);
                    std::string dir = path.parent_path().string();
                    std::string imgfile = (path.parent_path() / fileName).string();
                    // trimesh->setTexture(Image::fromFile(imgfile));
                }
            } else {
                continue;
            }
        } else {
            Assertion(numVerts > 0 && numFaces > 0, "numVerts and numFaces must be positive");

            float ff[3];
            //float tt[2];
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

    std::vector<ShapeGroup> ret;
    ret.emplace_back(tris, nullptr, nullptr);

    return std::move(ret);
}

std::vector<ShapeGroup> loadOBJ(const std::string& filename,
                                const Transform& objectToWorld) {
    // Load OBJ file with "tinyobjloader".
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string errors;
    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &errors, filename.c_str());
//    if (!errors.empty()) {
//        Warning(errors.c_str());
//    }

    if (!success) {
        FatalError("Failed to open OBJ file \"%s\" !!", filename.c_str());    
    }

    // Prepare return object.
    std::vector<ShapeGroup> groups;
    for (const auto &s : shapes) {
        std::vector<Point3d> positions;
        std::vector<Normal3d> normals;
        std::vector<Point2d> texcoords;

        bool hasNormal = true;
        bool hasTexcoord = true;
        for (const auto &index : s.mesh.indices) {
            Point3d position;
            Normal3d normal;
            Point2d texcoord;
        
            if (index.vertex_index >= 0) {
                position = Point3d(attrib.vertices[index.vertex_index * 3 + 0],
                                   attrib.vertices[index.vertex_index * 3 + 1],
                                   attrib.vertices[index.vertex_index * 3 + 2]);
            }

            if (index.normal_index >= 0) {
                normal = Normal3d(attrib.normals[index.normal_index * 3 + 0],
                                  attrib.normals[index.normal_index * 3 + 1],
                                  attrib.normals[index.normal_index * 3 + 2]);
            } else {
                hasNormal = false;
            }

            if (index.texcoord_index >= 0) {
                texcoord = Point2d(attrib.texcoords[index.texcoord_index * 2 + 0],
                                   attrib.texcoords[index.texcoord_index * 2 + 1]);
            } else {
                hasTexcoord = false;
            }

            positions.push_back(position);
            normals.push_back(normal);
            texcoords.push_back(texcoord);
        }

        std::vector<std::shared_ptr<Shape>> tris;
        for (int i = 0; i < positions.size(); i += 3) {
            std::shared_ptr<Shape> tri;
            if (hasNormal && hasTexcoord) {
                tri = std::make_shared<Triangle>(positions[i + 0], positions[i + 1], positions[i + 2],
                                                 normals[i + 0], normals[i + 1], normals[i + 2],
                                                 texcoords[i + 0], texcoords[i + 1], texcoords[i + 2], objectToWorld);
            } else if (hasNormal) {
                tri = std::make_shared<Triangle>(positions[i + 0], positions[i + 1], positions[i + 2],
                                                 normals[i + 1], normals[i + 1], normals[i + 2], objectToWorld);            
            } else {
                tri = std::make_shared<Triangle>(positions[i + 0], positions[i + 1], positions[i + 2], objectToWorld);
            }
            tris.push_back(tri);
        }

        std::shared_ptr<ImageTexture>    mapKd   = nullptr;
        std::shared_ptr<Texture<double>> bumpMap = nullptr;
        //if (!materials.empty()) {
        //    if (!materials[i].diffuse_texname.empty()) {
        //        Image image = Image::fromFile(materials[i].diffuse_texname);
        //        auto uvMap = std::shared_ptr<UVMapping2D>();
        //        mapKd = std::make_shared<ImageTexture>(image, uvMap, ImageWrap::Black);
        //    }

        //    if (!materials[i].bump_texname.empty()) {
        //        Image image = Image::fromFile(materials[i].bump_texname);
        //    }
        //}
        groups.emplace_back(tris, mapKd, bumpMap);
    }

    return std::move(groups);
}

}

}  // namespace spica
